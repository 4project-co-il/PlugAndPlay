#include <Arduino.h>
#include "PlugAndPlay.h"

// This example will use the 1RGB LED PnP module and blink it On/Off using a timer every second.

// EBF objects creation, should be global
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// In this example the serial interface is used for EBF error reporting, so it's advised to always
// have it initialized for debugging
PnP_Serial serial;

// Timer that will be used to toggle the LED on and off
EBF_Timer timer;

// The PnP hardwre module with the PRG LED
PnP_Module_1RGB_Led rgbLed;

// Timer callback function
void onTimer()
{
	// GetRBG returns 0 when the led is off (black color = 0x00000000)
	if (rgbLed.GetRGB() == 0) {
		// LED is OFF, turn it ON
		rgbLed.On();
		serial.println("ON");

		// You can use the SetRGB() function instead to light a specific LED color
		//rgbLed.SetRGB(50, 0, 30);
	} else {
		// LED is ON, turn it OFF
		rgbLed.Off();
		serial.println("OFF");
	}

	// EBF timers are one-time by design, so we need to start it again
	timer.Start();
}

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

	// Initialize the RGB LED.
	rgbLed.Init();

	// By default LED's color is set to white (R=100%, G=100%, B=100%)
	// You can uncomment and change the colors for the collowing SetRGB() function call
	// Last flag parameter specify if the LED should be kept off after changing the color
	// In that example it will be turned on by the timer
	//rgbLed.SetRGB(30, 0, 50, true);
	// Or use the single parameter SetRGB() function variant, where the parameter can pass a hex value in format of 0xRRGGBB
	//rgbLed.SetRGB(0x1E0032, true);

	// Initialize and start 1Hz (1000 mSec) timer
	timer.Init(onTimer, 1000);
	timer.Start();

	serial.println("Starting...");
}

void loop()
{
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
