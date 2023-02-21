#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;

DaisyPatchSM patch;
ReverbSc     reverb;

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
        OUT_L[i] = dryl + wetl;
        OUT_R[i] = dryr + wetr;
    }
}

int main(void)
{
    patch.Init();
    reverb.Init(patch.AudioSampleRate());
    patch.StartAudio(AudioCallback);
    while(1) {}
}
