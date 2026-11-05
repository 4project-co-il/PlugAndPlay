#include <Arduino.h>
#include "PlugAndPlay.h"

// This example will use the 1RGB LED PnP module and use internal animation mechanism to blink it twice, every 1 second

// EBF objects creation, should be global
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// In this example the serial interface is used for EBF error reporting, so it's advised to always
// have it initialized for debugging
PnP_Serial serial;

// Timer that will be used to initiate the blinking
EBF_Timer timer;

// The PnP hardwre module with the PRG LED
PnP_Module_1RGB_Led rgbLed;

// Timer callback function
void onTimer()
{
	serial.println("Blink twice");

	// Start the animation we prepared in setup()
	rgbLed.StartAnimation();

	// EBF timers are one-time by design, so we need to start it again
	timer.Start();
}

// All the setup should be done in that function
void setup()
{
	// Uncomment that line if you want to wait for establishment of the serial connection
	//while (!serial) {}

	// EBF error reporting interface should be set before the initialization
	EBF.SetErrorHandlerSerial(serial);

	// EBF is the first thing that should be initialized
	// The parameter specifies number of queue entries that are used to pass interrupt calls to normal run
	EBF.Init(16);

	// EBF is the first thing that should be initialized
	// The parameter specifies number of queue entries that are used to pass interrupt calls to normal run
	serial.Init();

	// Initialize and start 1Hz (1000 mSec) timer
	timer.Init(onTimer, 1000);
	timer.Start();

	// Initialize the RGB LED
	rgbLed.Init();

	// Set the color to RED, but don't light the LED yet, it will be done by the animation in the timer
	rgbLed.SetRGB(100, 0, 0, true);

	// Prepare the animation for short blink with timing 50mSec on, 200mSec off. Repeat 2 times.
	// Since SetFadingTime() wasn't called, the transition between ON and OFF states will be instant
	rgbLed.PrepareBlinkAnimation(PnP_Module_1RGB_Led::Time::TIME_50MS, PnP_Module_1RGB_Led::Time::TIME_200MS, 2);

	serial.println("Starting...");
}

void loop()
{
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}