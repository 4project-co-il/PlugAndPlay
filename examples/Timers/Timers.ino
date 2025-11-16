#include <Arduino.h>
#include "PlugAndPlay.h"

// This is an example for basic Plug-n-Play code strucure
// The code will initialize 2 timers with different intervals.
// One timer will print a counter value to the serial interface
// Another timer will utilize LED's internal Blink() functionality
// for fast blinking during 1 second and slow blinking during next second

// Objects creation
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Timer objects
EBF_Timer ledTimer;
EBF_Timer printTimer;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// In this example the serial interface is used for EBF error reporting, so it's advised to always
// have it initialized for debugging
PnP_Serial serial;

// Status LED on the development board
PnP_StatusLed led;

bool fastBlink = false;

// This function will be called every time the LED_TIMER expires
void onLedTimer()
{
	// If we're in fast blinking mode
	if (fastBlink) {
		// Blink the status LED with 50mSec ON state and 50mSec OFF
		led.Blink(50, 50);
	} else {
		// Blink the status LED with 20mSec ON state and 230mSec OFF
		led.Blink(20, 230);
	}

	// Change the blinking mode
	fastBlink = !fastBlink;

	// Timers are one-shot by default. Start it again for next run
	ledTimer.Start();
}

int counter = 0;

// This function will be called every time the PRINT_TIMER expires
void onPrintTimer()
{
	// Printing is done the same way as you would do with the Arduino's Serial object
	serial.print("Counter: ");
	serial.println(counter);

	counter++;

	// Timers are one-shot by default. Start it again for next print
	printTimer.Start();
}

// All the setup should be done in that function
void setup()
{
	// Uncomment that line if you want to wait for establishment of the serial connection
	//while (!serial) {}

	// EBF error reporting interface should be set before the initialization
	EBF.SetErrorHandlerSerial(serial);

	// EBF is the first thing that should be initialized, with the maximum timers to be used
	// The parameter specifies how many interrupt events can be queued in transition
	// between the interrupt execution (ISR) and the normal run loop.
	// For light applications without fast events and long execution, number 4 might be enough.
	// For applications with fast burst of events, or long execution for some of them, might need a larger number
	EBF.Init(16);

	// Initialize serial interface object
	serial.Init();

	// Initialize the status LED
	led.Init();

	// Initialize the LED_TIMER to 1000mSec (1 second)
	// The onLedTimer function will be called every time the LED_TIMER expires
	ledTimer.Init(onLedTimer, 1000);
	// Start the timer
	ledTimer.Start();

	// Initialize the PRINT_TIMER to 3000mSec (3 seconds)
	// The onPrintTimer function will be called every time the PRINT_TIMER expires
	printTimer.Init(onPrintTimer, 3000);
	// Start the timer
	printTimer.Start();

	serial.println("Starting...");
}

void loop() {
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
