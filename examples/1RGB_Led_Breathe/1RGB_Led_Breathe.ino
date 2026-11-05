#include <Arduino.h>
#include "PlugAndPlay.h"

// This example will use the 1RGB LED PnP module and will run automatic "breath" animation,
// which is performed completely by the module without microcotroller intervention

// EBF objects creation, should be global
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// In this example the serial interface is used for EBF error reporting, so it's advised to always
// have it initialized for debugging
PnP_Serial serial;

// The PnP hardwre module with the PRG LED
PnP_Module_1RGB_Led rgbLed;

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

	// Initialize serial interface object
	serial.Init();

	// Initialize the RGB LED
	rgbLed.Init();

	// Set fading time to 300mSec
	// This setting creates the gradual raise or decrease in LED intensity
	// the time specifies how long it will take to transit between the initial and destination levels
	rgbLed.SetFadingTime(PnP_Module_1RGB_Led::Time::TIME_300MS);

	// Set the color to pink, but do not display it yet. It will be done by the animation engine.
	// The parameters are in percent of max intensity, from 0% to 100%
	rgbLed.SetRGB(30, 0, 50, true);

	// Prepare amination of breathing pattern with the following timing:
	// fade-in (300mSec) + on time (50mSec) + fade-out (300mSec) + off time (2sec)
	// the fading value is taken from the SetFadingTime() function call
	// Last parameter sets number of repetitions. 15 means repeat forever.
	rgbLed.PrepareBlinkAnimation(PnP_Module_1RGB_Led::Time::TIME_50MS, PnP_Module_1RGB_Led::Time::TIME_2000MS, 15);

	// Starting the animation
	rgbLed.StartAnimation();

	serial.println("Starting...");
}

void loop()
{
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
