#include "daisy_seed.h"
#include "daisysp.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;

// Declare a DaisySeed object called hardware
DaisySeed hw;

struct CD4017 {
    GPIO    clock;
    GPIO    reset;
    int     position = 0;
};
CD4017 counter;



void AdvanceCounter()
{
    counter.clock.Write(true);
    counter.clock.Write(false);
    counter.position += 1;
}

void ResetCounter()
{
    counter.reset.Write(true);
    counter.reset.Write(false);
    counter.position = 0;
}

void InitCounter()
{
    counter.clock.Init(D0, GPIO::Mode::OUTPUT);
    counter.reset.Init(D1, GPIO::Mode::OUTPUT);
    ResetCounter();
}

int main(void)
{
    // Configure and Initialize the Daisy Seed
    hw.Configure();
    hw.Init();
    InitCounter();

    // Loop forever
    for(;;) {

        // Counter logic - advance but wrap around on step 8.
        if ( counter.position == 8 ) {
            // Reset 4017 and counter position
            ResetCounter();
            // Advance
            AdvanceCounter();
        }
        else if ( counter.position < 8 ) {
            // Advance the counter
            AdvanceCounter();
        }

        // Wait for next step
        System::Delay(600);

    }
}
