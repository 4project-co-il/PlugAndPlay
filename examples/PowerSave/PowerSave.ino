#include <Arduino.h>
#include "PlugAndPlay.h"

// This is an example for basic Plug-n-Play code strucure
// This example will blink the status led on the PnP development board
// And enter deep sleep mode mode between the transitions

// Objects creation
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// In this example the serial interface is used for EBF error reporting, so it's advised to always
// have it initialized for debugging
PnP_Serial serial;

// Status LED on the development board
PnP_StatusLed led;

// Timers enumeration
enum {
	LED_TIMER = 0,

	NUMBER_OF_TIMERS
};

// This function will be called every time the LED_TIMER expires
void onLedTimer()
{
	// If LED is off
	if (led.GetValue() == 0) {
		// Turn it on for 100mSec
		led.On();
		EBF.StartTimer(LED_TIMER, 100);
	} else {
		// Otherwise, turn it off for 5000 mSec
		led.Off();
		EBF.StartTimer(LED_TIMER, 5000);
	}
}

// All the setup should be done in that function
void setup()
{
	// Uncomment that line if you want to wait for establishment of the serial connection
	//while (!serial) {}

	// EBF error reporting interface should be set before the initialization
	EBF.SetErrorHandlerSerial(serial);

	// EBF is the first thing that should be initialized, with the maximum timers to be used
	// First parameter is the number of timers for that program.
	// Second parameter specifies how many interrupt events can be queued in transition
	// between the interrupt execution (ISR) and the normal run loop.
	// For light applications without fast events and long execution, number 4 might be enough.
	// For applications with fast burst of events, or long execution for some of them, might need a larger number
	EBF.Init(NUMBER_OF_TIMERS, 16);

	// Set DEEP sleep mode where the development board will try to save as much power as possible
	EBF.SetSleepMode(EBF_SLEEP_DEEP);

	// Initialize serial interface object
	serial.Init();

	// Initialize the status LED
	led.Init();

	// Initialize the LED_TIMER to 1000mSec (1 second)
	// This is only for the first run since the onLedTimer will restart it for different dirations
	EBF.InitTimer(LED_TIMER, onLedTimer, 1000);
	// Start the timer
	EBF.StartTimer(LED_TIMER);

	serial.println("Starting...");
}

void loop() {
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
