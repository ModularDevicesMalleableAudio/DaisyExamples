#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hardware;

struct LEDStruct {
    Led        led;
    float      led_value;
};
LEDStruct leds[8];
int position = 7;

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

    // Loop forever
    for(;;){
        // Turn current led brightness to 0
        leds[position].led_value = 0.f;
        // increment position, and wrap round if necessary
        position += 1;
        position = position % 8;
        // Set the next led brightness to full
        leds[position].led_value = 6.341325705384997f;

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
