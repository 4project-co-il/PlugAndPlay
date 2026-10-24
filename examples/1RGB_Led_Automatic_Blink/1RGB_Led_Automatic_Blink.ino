#include <Arduino.h>
#include "PlugAndPlay.h"

// EBF objects creation, should be global
EBF_Core EBF;

PnP_Module_1RGB_Led rgbLed;

EBF_Timer timer;

PnP_Serial serial;

void onTimer()
{
	serial.println("Blink twice");

	// Animate short blink with timing 50mSec on, 200mSec off. Repeat 2 times.
	rgbLed.Animate(PnP_Module_1RGB_Led::Time::TIME_50MS, PnP_Module_1RGB_Led::Time::TIME_200MS, 2);

	// Start the timer again
	timer.Start();
}

void setup()
{
	// Uncomment that line if you want to wait for establishment of the serial connection
	//while (!serial) {}

	// EBF error reporting interface should be set before the initialization
	EBF.SetErrorHandlerSerial(serial);

	// EBF is the first thing that should be initialized, with the maximum timers to be used
	EBF.Init(16);

	// Initialize serial communication
	serial.Init();

	// Initialize and start 1Hz timer
	timer.Init(onTimer, 1000);
	timer.Start();

	// Initialize the RGB LED
	rgbLed.Init();

	// Set the color to RED, but don't light the led yet
	rgbLed.SetRGB(100, 0, 0, true);

	serial.println("Starting...");
}

void loop()
{
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}