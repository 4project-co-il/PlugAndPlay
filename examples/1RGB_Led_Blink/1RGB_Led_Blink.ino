#include <Arduino.h>
#include "PlugAndPlay.h"

// EBF objects creation, should be global
EBF_Core EBF;

PnP_Module_1RGB_Led rgbLed;

EBF_Timer timer;

PnP_Serial serial;

void onTimer()
{
	// GetRBG returns 0 when the led is off (black color = 0x00000000)
	if (rgbLed.GetRGB() == 0) {
		// LED is OFF, turn it ON
		rgbLed.On();
		serial.println("ON");

		// You can also use the SetRGB() function to light a specific LED color
	} else {
		// LED is ON, turn it OFF
		rgbLed.Off();
		serial.println("Off");
	}

	// Start the timer
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

	serial.println("Starting...");
}

void loop()
{
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
