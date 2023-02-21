#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;
using namespace std;

DaisyPatchSM patch;
ReverbSc     reverb;
Switch       toggle;
Switch       button;

struct envStruct
{
    AdEnv     env;
    float     envSig;
    bool      DuckingEnabled;
};

envStruct     envelope;

int counters[2] = {0, 0 };
int divisions[2] = {2, 3 };

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    patch.ProcessAllControls();

    /** Update time from knob CV_1 (0, 1) */
    float time_knob = patch.GetAdcValue(CV_1);
    /** Read from CV_5 (-1, 1) and add one so its (0, 2) */
    float time_cv   = patch.GetAdcValue(CV_5) + 1;
    /** Combine and set max and min for time value */
    float time      = fmap((time_knob * time_cv), 0.3f, 0.99f);

    /** Update dampening from knob CV_2 (0, 1) */
    float damp_knob = patch.GetAdcValue(CV_2);
    /** Read from CV_6 (-1, 1) and add one so its (0, 2) */
    float damp_cv   = patch.GetAdcValue(CV_6) + 1;
    /** Combine and set max and min for damp value with log response */
    float damp      = fmap(
        (damp_knob * damp_cv),
        1000.f,
        19000.f,
        Mapping::LOG);

    /** Dry/Wet for the Reverb */
    float dry_wet = fmap(
        patch.GetAdcValue(CV_3),
        0.f,
        1.f,
        Mapping::LINEAR);

    reverb.SetFeedback(time);
    reverb.SetLpFreq(damp);

    /** For each sample, calculate the output */
    for(size_t i = 0; i < size; i++)
    {
        float dryl  = IN_L[i] * (1.f - dry_wet);
        float dryr  = IN_R[i] * (1.f - dry_wet);
        float sendl = IN_L[i] * dry_wet;
        float sendr = IN_R[i] * dry_wet;

        float wetl, wetr;
        reverb.Process(sendl, sendr, &wetl, &wetr);

        if (envelope.DuckingEnabled)
        {
            envelope.envSig = envelope.env.Process();
            OUT_L[i] = (dryl + wetl) * envelope.envSig;
            OUT_R[i] = (dryr + wetr) * envelope.envSig;
        }
        else
        {
            OUT_L[i] = (dryl + wetl);
            OUT_R[i] = (dryr + wetr);
        }
    }
}

void InitEnvelope(float samplerate)
{
    envelope.DuckingEnabled = true;
    envelope.envSig = 0.f;
    //envelope values and Init
    envelope.env.Init(samplerate);
    envelope.env.SetMax(0.f);
    envelope.env.SetMin(1.f);
    envelope.env.SetCurve(2.f);
    envelope.env.SetTime(ADENV_SEG_ATTACK, 0.015f);
    envelope.env.SetTime(ADENV_SEG_DECAY, 0.5f);
}

int GetOutputState(int counter, int state)
{
    if (counter == 0)
    {
        return state;
    }
    return 0;
}

int main()
{
    patch.Init();
    reverb.Init(patch.AudioSampleRate());
    InitEnvelope(patch.AudioSampleRate());
    button.Init(daisy::patch_sm::DaisyPatchSM::B7);
    toggle.Init(daisy::patch_sm::DaisyPatchSM::B8);
    patch.StartAudio(AudioCallback);
    while(1) {

        /** Read from CV_4 (-1, 1) and scale to (0.00001, 0.1) */
        float env_decay = fmap(
            patch.GetAdcValue(CV_4),
            .005f,
            0.5f,
            Mapping::LINEAR);
        envelope.env.SetTime(ADENV_SEG_DECAY, env_decay);

        /** Debounce the switches */
        toggle.Debounce();
        button.Debounce();

        /** Turn ducking on/off */
        if (button.RisingEdge())
        {
            if (envelope.DuckingEnabled)
            {
                envelope.DuckingEnabled = false;
            }
            if (not envelope.DuckingEnabled)
            {
                envelope.DuckingEnabled = true;
            }
        }

        /** Get the current toggle state and relevant clock divisions */
        bool toggle_state = toggle.Pressed();
        if (toggle_state)
        {
            divisions[0] = 2;
            divisions[1] = 3;
//            envelope.DuckingEnabled = true;
        }
        else
        {
            divisions[0] = 4;
            divisions[1] = 6;
//            envelope.DuckingEnabled = false;
        }

        /** Get the current gate in state */
        bool state1 = patch.gate_in_1.State();
        bool state2 = patch.gate_in_2.State();

        /** Advance counters */
        if (patch.gate_in_1.Trig())
        {
            counters[0] += 1;
            counters[0] = counters[0] % divisions[0];
            if (envelope.DuckingEnabled)
            {
                envelope.env.Trigger();
            }
        }
        if (patch.gate_in_2.Trig())
        {
            counters[1] += 1;
            counters[1] = counters[1] % divisions[1];
        }

        /** Get the current gate out state based on the counters */
        int OutputState1 = GetOutputState(counters[0], state1);
        int OutputState2 = GetOutputState(counters[1], state2);
        int JointOutputState = std::max(OutputState1, OutputState2);
        float led_voltage = JointOutputState * 5.0f;
        if (OutputState1 > 0 and  !envelope.DuckingEnabled)
        {
            patch.WriteCvOut(
                CV_OUT_1,
                patch.GetRandomFloat(0.0, 5.0)
                );
        }
        if (envelope.DuckingEnabled)
        {
            patch.WriteCvOut(
                CV_OUT_1,
                envelope.envSig
            );
        }
        /** Send CV out */
        dsy_gpio_write(&patch.gate_out_1, OutputState1);
        dsy_gpio_write(&patch.gate_out_2, OutputState2);
        patch.WriteCvOut(CV_OUT_2, led_voltage);
    }
}
