#include <Arduino.h>
#include "PlugAndPlay.h"

// This example will use the STTS22H temperature sensor to show the measured temperature
// on SparkFun's QWIIC LCD connected via QWIIC<->PnP adapter.
// In addition, the sensor will fire an interrupt when measured temperature will be higher
// than specified value, which will blink the status LED and turn the LCD red.

// Objects creation
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// In this example the serial interface is used for EBF error reporting, so it's advised to always
// have it initialized for debugging
PnP_Serial serial;

// Status LED on the development board
PnP_StatusLed led;

// SparkFun's QWIIC SerLCD connected to the PnP system using QWIIC<->PnP adapter with needed EEPROM values
// PnP_Module_Seeed_Monochrome_GROVE_16x2_LCD class can be used the same way, except the color changing functions
PnP_Module_SparkFun_QWIIC_SerLCD lcd;

// STTS22H temperature sensor module
PnP_Module_STTS22H_TemperatureSensor tempSensor;

enum {
	PRINT_TIMER = 0,
	CLEAR_TIMER,

	NUMBER_OF_TIMERS
};

// This function will be called when PRINT_TIMER expires
void onPrintTimer()
{
	char str[16];

	// Formatting the string to be printed
	sprintf(str, "Temp: %0.2f   ", tempSensor.GetValueC());

	// Print the measurement on the first row
	lcd.SetCursor(0, 0);
	lcd.print(str);

	// Print the measurement to serial as well
	//serial.println(str);

	// Timers are always one-shot, restart it for the next print
	EBF.StartTimer(PRINT_TIMER);
}

// This function will be called when CLEAR_TIMER expires
void onClearTimer()
{
	// Clear the second row of the LCD by writing 16 spaces
	lcd.SetCursor(0, 1);
	lcd.print("                ");

	// Set the backgroud color to white
	lcd.SetBacklight(0xFF, 0xFF, 0xFF);

	// Turn off the status led
	led.Off();
}

// This function will be called when temperature sensor will report high temperature
void onHighTemp()
{
	// Print on the second row
	lcd.SetCursor(0, 1);
	lcd.print("!!! TOO HOT !!!");

	// Set the background color to red
	lcd.SetBacklight(0xFF, 0, 0);

	// If led is already blinking, don't start the blinking again, that visibly interrupts the sequence
	if (led.GetValue() == 0) {
		// Blink the status led
		led.Blink(100, 100);
	}

	// Restart the clear timer.
	// Since the measurement is done every 1 second and the clear timer is set to 3 seconds,
	// it will be restarted before it will get to the expiration time.
	// The clear timer callback will execute only when the temperature will get below the
	// configured threshold
	EBF.RestartTimer(CLEAR_TIMER);
}

// All the setup should be done in that function
void setup()
{
	// Uncomment that line if you want to wait for establishment of the serial connection
	//while (!serial) {}

	// EBF error reporting interface should be set before the initialization
	EBF.SetErrorHandlerSerial(serial);

	// EBF is the first thing that should be initialized
	// Second parameter specifies number of queue entries that are used to pass interrupt calls to normal run
	EBF.Init(NUMBER_OF_TIMERS, 16);

	// Initialize the status led
	led.Init();

	// Initialize the print timer to 1 seconds
	EBF.InitTimer(PRINT_TIMER, onPrintTimer, 1000);
	// Start the timer
	EBF.StartTimer(PRINT_TIMER);

	// Initialize the alarm timer to 3 seconds, which will be restarted every time the high temperature alarm will be executed
	// Since the timeout is longer than the measurement time (1 second), the timer will never expire, unless the temeprature
	// will not go back under the threshold, so clear timer could finish its counting and call for the onClearTimer callback
	EBF.InitTimer(CLEAR_TIMER, onClearTimer, 3000);

	// Initialize serial interface object
	serial.Init();

	// Initialize the LCD module
	lcd.Init();

	// Clearing the display and setting the backgrount to white
	lcd.Clear();
	lcd.SetBacklight(0xFF, 0xFF, 0xFF);

	// Initialize the temperature sensor module and set its measurement period to 1 second
	tempSensor.Init();
	tempSensor.SetOperationMode(EBF_STTS22H_TemperatureSensor::MODE_1HZ);
	// Setting the high temperature threshold to 30 degC
	tempSensor.SetThresholdHigh(30.0);
	// Registering the callback function for the high temperature threshold
	tempSensor.SetOnThresholdHigh(onHighTemp);

	serial.println("Starting...");
}

void loop() {
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
