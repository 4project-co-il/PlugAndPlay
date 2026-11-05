#include <Arduino.h>
#include "PlugAndPlay.h"

// This example will use the 1RGB LED PnP module and will run the "police light" animation,
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

	// EBF is the first thing that should be initialized
	// The parameter specifies number of queue entries that are used to pass interrupt calls to normal run
	serial.Init();

	// Initialize the RGB LED
	rgbLed.Init();

	// Prepare the police light animation (red and blue) with default parameters
	rgbLed.PreparePoliceAnimation();

	// Default parameters are:
	// ON time for every flash = 50mSec
	// OFF time between every flash = 50mSec
	// Number of flashes for every color sequence = 5
	// Rest time between the colors = 100mSec
	// Intensity of the LED = 100%

	// You can pass other parameter values by calling the function:
	//rgbLed.PreparePoliceAnimation(PnP_Module_1RGB_Led::Time::TIME_50MS, PnP_Module_1RGB_Led::Time::TIME_50MS, 5, PnP_Module_1RGB_Led::Time::TIME_100MS, 100.0);

	// Start the animation, that will repeat infinitely
	rgbLed.StartAnimation();

	// You can stop the animation by calling the StopAnimation() function anywhere in your code
	//rgbLed.StopAnimation();

	serial.println("Starting...");
}

void loop()
{
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}