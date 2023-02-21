#include "daisy_seed.h"
#include "daisysp.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisySeed object called hardware
DaisySeed hardware;

struct LEDStruct {
    Led        led;
    float      led_value;
};
LEDStruct leds[8];
int position = 7;

//static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
//                          AudioHandle::InterleavingOutputBuffer out,
//                          size_t                                size)
//{
//    for(size_t i = 0; i < size; i += 2)
//    {
//        for(auto & led : leds) {
//            led.led_value = fmap(lfo.lfo.Process(), 0.f, 6.341325705384997f);
//        }
//    }
//
//    for(auto & led : leds)
//    {
//        // Set the onboard LED to the value we read from the knob
//        lfo.led.Set(lfo.led_value);
//        //Update the led to reflect the set value
//        lfo.led.Update();
//    }
//}

void InitLEDs()
{
    // Initialize leds on pins 18 to 25.
    leds[0].led.Init(hardware.GetPin(25), false);
    leds[1].led.Init(hardware.GetPin(24), false);
    leds[2].led.Init(hardware.GetPin(23), false);
    leds[3].led.Init(hardware.GetPin(22), false);
    leds[4].led.Init(hardware.GetPin(21), false);
    leds[5].led.Init(hardware.GetPin(20), false);
    leds[6].led.Init(hardware.GetPin(19), false);
    leds[7].led.Init(hardware.GetPin(18), false);
}

int main(void)
{
    // Configure and Initialize the Daisy Seed
    hardware.Configure();
    hardware.Init();
    InitLEDs();

//    // This is our ADC configuration
//    AdcChannelConfig adcConfig;
//    // Configure pin 21 as an ADC input. This is where we'll read the knob.
//    adcConfig.InitSingle(hardware.GetPin(21));
//
//    // Initialize the adc with the config we just made
//    hardware.adc.Init(&adcConfig, 1);
//    // Start reading values
//    hardware.adc.Start();
//    // Start LFO in the AudioCallback
//    hardware.StartAudio(AudioCallback);

    // Loop forever
    for(;;){
        // Turn current led brightness to 0
        leds[position].led_value = 0.f;
        // increment position, and wrap round if neccesary
        position += 1;
        position = position % 8;
        // Set the next led brightness to full
        leds[position].led_value = 6.341325705384997f;

//        float knob_val = fmap(hardware.adc.GetFloat(0),0.1f,1.f);
//        for (auto & led: leds) {
//            lfo.lfo.SetFreq(knob_val / lfo.freq_div);
//        }
        // Update all led statuses
        for(auto & led : leds)
        {
            // Set the onboard LED to the value we read from the knob
            led.led.Set(led.led_value);
            //Update the led to reflect the set value
            led.led.Update();
        }
        System::Delay(100);
    }
}
