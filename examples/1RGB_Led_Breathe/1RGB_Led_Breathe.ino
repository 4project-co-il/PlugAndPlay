#include <Arduino.h>
#include "PlugAndPlay.h"

// EBF objects creation, should be global
EBF_Core EBF;

PnP_Module_1RGB_Led rgbLed;

PnP_Serial serial;

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

	// Initialize the RGB LED
	rgbLed.Init();

	// Set fading time to 300mSec
	rgbLed.SetFadingTime(PnP_Module_1RGB_Led::Time::TIME_300MS);

	// Set the color to pink, but do not display it yet.
	// The parameters are in percent of max intensity, from 0% to 100%
	rgbLed.SetRGB(30, 0, 50, true);

	// Aminate breathing pattern with the following timing: fade-in (300mSec) + on time (50mSec) + fade-out (300mSec) + off time (2sec)
	// Last parameter sets number of repetitions. 15 means repeat forever.
	rgbLed.Animate(PnP_Module_1RGB_Led::Time::TIME_50MS, PnP_Module_1RGB_Led::Time::TIME_2000MS, 15);

	serial.println("Starting...");
}

void loop()
{
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
