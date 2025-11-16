#include <Arduino.h>
#include "PlugAndPlay.h"

// This example will use the 2Buttons PnP module and will print
// buttons states to SparkFun QWIIC LCD connected via QWIIC<->PnP adapter

// Objects creation
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Timers objects
EBF_Timer clearRow0Timer;
EBF_Timer clearRow1Timer;
EBF_Timer lightsOffTimer;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// In this example the serial interface is used for EBF error reporting, so it's advised to always
// have it initialized for debugging
PnP_Serial serial;

// SparkFun's QWIIC SerLCD connected to the PnP system using QWIIC<->PnP adapter with needed EEPROM values
// PnP_Module_Seeed_Monochrome_GROVE_16x2_LCD class can be used the same way, except the color changing functions
PnP_Module_SparkFun_QWIIC_SerLCD lcd;

// The PnP hardware module with 2 buttons
PnP_Module_2ButtonsInput buttonsModule;

// Buttons interface is used to receive buttons implementation
PnP_ButtonInterface buttons[2];


// This function will be called when a button is pressed
void onButtonPress()
{
	PnP_ButtonInterface* pCurrentButton;

	pCurrentButton = (PnP_ButtonInterface*)buttonsModule.GetCurrentInterface();

	serial.print("Button ");
	serial.print(pCurrentButton->GetUserData());
	serial.println(" pressed");

	// We have 2 buttons, each status will be printed on its own row
	lcd.SetCursor(0, pCurrentButton->GetUserData());

	lcd.print("# ");
	lcd.print(pCurrentButton->GetUserData());
	lcd.print(" pressed     ");

	// Clear the relevant row after the timer value
	if (pCurrentButton->GetUserData() == 0) {
		// Set green background for button 0
		lcd.SetBacklight(0, 0xFF, 0);

		clearRow0Timer.Restart();
	} else {
		// Set blue background for button 1
		lcd.SetBacklight(0, 0, 0xFF);

		clearRow1Timer.Restart();
	}

	// Stop the shutdown timer when a button is pressed
	lightsOffTimer.Stop();
}

// This function will be called when a button is released
void onButtonRelease()
{
	PnP_ButtonInterface* pCurrentButton;

	pCurrentButton = (PnP_ButtonInterface*)buttonsModule.GetCurrentInterface();

	serial.print("Button ");
	serial.print(pCurrentButton->GetUserData());
	serial.println(" released");

	// We have 2 buttons, each status will be printed on its own row
	lcd.SetCursor(0, pCurrentButton->GetUserData());

	lcd.print("# ");
	lcd.print(pCurrentButton->GetUserData());
	lcd.print(" released    ");

	// Change to white background when any button is released
	lcd.SetBacklight(0xFFFFFF);

	// Clear the relevant row after the timer value
	if (pCurrentButton->GetUserData() == 0) {
		clearRow0Timer.Restart();
	} else {
		clearRow1Timer.Restart();
	}

	// Buttons are working
	if (buttons[0].IsPressed() == 0 && buttons[1].IsPressed() == 0) {
		// Shutdown the lights if both buttons are released
		lightsOffTimer.Restart();
	}
}

// This functino will be called when a button is pressed long (3 sec by default, can be changed by buttons.SetLongPressTime() function call)
void onButtonLong()
{
	PnP_ButtonInterface* pCurrentButton;

	pCurrentButton = (PnP_ButtonInterface*)buttonsModule.GetCurrentInterface();

	serial.print("Button ");
	serial.print(pCurrentButton->GetUserData());
	serial.println(" long pressed");

	// We have 2 buttons, each status will be printed on its own row
	lcd.SetCursor(0, pCurrentButton->GetUserData());

	lcd.print("# ");
	lcd.print(pCurrentButton->GetUserData());
	lcd.print(" long press  ");

	// Change to red background when any button is long pressed
	lcd.SetBacklight(0xFF, 0, 0);

	// Clear the relevant row after the timer value
	if (pCurrentButton->GetUserData() == 0) {
		clearRow0Timer.Restart();
	} else {
		clearRow1Timer.Restart();
	}
}

// This function will be called when TIMER_CLEAR_ROW0 timer expires
void onClearRow0Timer()
{
	// Clear LCD display
	lcd.SetCursor(0, 0);
	lcd.print("                ");

	serial.println("Clear row 0");
}

// This function will be called when TIMER_CLEAR_ROW1 timer expires
void onClearRow1Timer()
{
	// Clear LCD display
	lcd.SetCursor(0, 1);
	lcd.print("                ");

	serial.println("Clear row 1");
}

// This function will be called when TIMER_LIGHTS_OFF timer expires
void onLightsOffTimer()
{
	// Set the backlight color to black
	lcd.SetBacklight(0);

	serial.println("Lights off");
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

	// Initialize the clear timers to 1 seconds each.
	// In this example the timers are re-initialized after every button event to clear the relevant row on the LCD
	// meaning they are stopped and started again for the set timeout
	clearRow0Timer.Init(onClearRow0Timer, 1000);
	clearRow1Timer.Init(onClearRow1Timer, 1000);
	// Lights off timer is longer than the others, so the printing and cleaning event will be visible before it happens
	lightsOffTimer.Init(onLightsOffTimer, 10000);

	// Initialize serial interface object
	serial.Init();

	// Initialize the LCD module
	lcd.Init();

	// Clearing the display and setting the backgrount to black
	lcd.Clear();
	lcd.SetBacklight(0);

	// Initialize 2 buttons module
	buttonsModule.Init();

	// Assign the logic interfaces to the hardware module
	buttonsModule.AssignInterface(0, buttons[0]);
	buttonsModule.AssignInterface(1, buttons[1]);

	// Set the same callback functions to both buttons interfaces
	// Different callbacks might be used, but we can determine which button caused the event
	buttons[0].SetOnPress(onButtonPress);
	buttons[0].SetOnRelease(onButtonRelease);
	buttons[0].SetOnLongPress(onButtonLong);

	buttons[1].SetOnPress(onButtonPress);
	buttons[1].SetOnRelease(onButtonRelease);
	buttons[1].SetOnLongPress(onButtonLong);

	// We will assign numbers to the buttons as a user data. It can be any 32bit number, even a pointer to a function or an object
	// In this example we will use the UserData to print which button was pressed and set the LCD background color accordingly
	buttons[0].SetUserData(0);
	buttons[1].SetUserData(1);

	serial.println("Starting...");
}

void loop() {
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
