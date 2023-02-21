#include "daisy_seed.h"
#include "daisysp.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisySeed object called hardware
DaisySeed hardware;

struct LFOStruct {
    Led        led;
    Oscillator lfo;
    float      led_value;
    float      freq_div;
};
LFOStruct lfos[2];


static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    for(size_t i = 0; i < size; i += 2)
    {
        for(auto & lfo : lfos) {
            lfo.led_value = fmap(lfo.lfo.Process(), 0.f, 6.341325705384997f);
        }
    }

    for(auto & lfo : lfos)
    {
        // Set the onboard LED to the value we read from the knob
        lfo.led.Set(lfo.led_value);
        //Update the led to reflect the set value
        lfo.led.Update();
    }
}

void InitLFOs()
{
    // Second LFO is half speed of the first one
    lfos[0].freq_div = 1.f;
    lfos[1].freq_div = 2.f;

    for(auto & lfo : lfos)
    {
        // Initialise & set parameters for lfo
        lfo.lfo.Init(hardware.AudioSampleRate());
        lfo.lfo.SetWaveform(lfo.lfo.WAVE_SIN);
        lfo.lfo.SetAmp(1.f);
        lfo.lfo.SetFreq(0.1f / lfo.freq_div);
    }

    // Initialize leds on pins 28 & 23.
    lfos[0].led.Init(hardware.GetPin(28), false);
    lfos[1].led.Init(hardware.GetPin(23), false);
}

int main(void)
{
    // Configure and Initialize the Daisy Seed
    hardware.Configure();
    hardware.Init();
    InitLFOs();

    // This is our ADC configuration
    AdcChannelConfig adcConfig;
    // Configure pin 21 as an ADC input. This is where we'll read the knob.
    adcConfig.InitSingle(hardware.GetPin(21));

    // Initialize the adc with the config we just made
    hardware.adc.Init(&adcConfig, 1);
    // Start reading values
    hardware.adc.Start();
    // Start LFO in the AudioCallback
    hardware.StartAudio(AudioCallback);
    // Loop forever
    for(;;){
        float knob_val = fmap(hardware.adc.GetFloat(0),0.05f,2.f);
        for (auto & lfo: lfos) {
            lfo.lfo.SetFreq(knob_val / lfo.freq_div);
        }
        System::Delay(0.5f);
    }
}
