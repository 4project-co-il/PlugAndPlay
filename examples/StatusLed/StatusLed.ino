#include <Arduino.h>
#include "PlugAndPlay.h"

// This is an example for basic Plug-n-Play code strucure
// This example will blink the status led on the PnP development board in 1Hz intervals

// Objects creation
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Timer object
EBF_Timer ledTimer;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// In this example the serial interface is used for EBF error reporting, so it's advised to always
// have it initialized for debugging
PnP_Serial serial;

// Status LED on the development board
PnP_StatusLed led;

// This function will be called every time the LED_TIMER expires
void onLedTimer()
{
	// If LED is off
	if (led.GetValue() == 0) {
		// Turn it on
		led.On();
	} else {
		// Otherwise, turn it off
		led.Off();
	}

	// Timers are one-shot by default. Start it again for next run
	ledTimer.Start();
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

	serial.println("Starting...");
}

void loop() {
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
