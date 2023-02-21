#include "daisy_seed.h"
#include "daisysp.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisySeed object called hardware
DaisySeed hardware;

struct CD4017 {
    GPIO    clock;
    GPIO    reset;
    int     position = 0;
};
CD4017 counter;

int main(void)
{
    // Configure and Initialize the Daisy Seed
    hardware.Configure();
    hardware.Init();
    counter.clock.Init(D1, GPIO::Mode::OUTPUT);
    counter.reset.Init(D2, GPIO::Mode::OUTPUT);

    // Loop forever
    for(;;) {
        // Increment position, and wrap round if necessary, increment cd4017
        counter.position += 1;
        if ( counter.position > 7 ) {
            counter.position = 0;
            counter.reset.Write(True);
        }
        // If position is 0 then cd4017 output 0 is already high
        // Otherwise, we set the next output to be high
        if ( counter.position > 0 ) {
            counter.clock.Write(True);
        }

        // set clock to low, set reset to low if we have just reset
        counter.clock.Write(False);
        if ( counter.position == 0 ) {
            counter.reset.Write(False);
        }

        System::Delay(100);
    }
}
