#include <Arduino.h>
#include "PlugAndPlay.h"

// This example will use the 8Input PnP module and will print its input changes to the serial monitor
// The example will use PnP_BasicInputInterface for input change callbacks to present the flexibility
// of input interfaces concept

// Objects creation
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// In this example the serial interface is used for EBF error reporting, so it's advised to always
// have it initialized for debugging
PnP_Serial serial;

// The PnP hardware module with 8 inputs
PnP_Module_8Inputs inputModule;

// This example will use PnP_BasicInputInterface for callbacks, which provides the same OnChange functionality
// as the input module code, but adds flexibility of having multiple inputs, that might be connected to
// different physical input module.

// Since different devices might be connected to the module, you might want to use the corresponding interface
// implementation for that to have a more logical APIs and a specific additional functionality.
// Examples for such interfaces are:
// PnP_BasicInputInterface	- provides the same OnChange callbacks as the basic input module, but with a flexibility
// 							  of managing multiple inputs, even connected to different input modules.
// PnP_ButtonInterface		- privides button interface implementation with Pressed/Released and LongPressed APIs
// PnP_SwitchInterface		- provides switch interface implementation with On/Off APIs

// Define an array of basic input interfaces, according to the module inputs
PnP_BasicInputInterface inputs[inputModule.numberOfInputs];

// This function will be called when an input change will be detected
// In this example we use the same callback function for all the inputs and determine which line caused the event
// by calling the GetUserData() function. That user data is assigned during the setup and in that example just
// store the number of the input line for the module we used. We could store a pointer to a data structure that should
// be used when an input changes, or any other 32bit value instead.
// GetLastValue function returns the value that was registered when the change was detected.
// It is more efficient and more logical to use GetLastValue in that case since we're processing the change event and
// it's more logical to use the value of the lines as it was during the detection of the change.
// GetValue() can be used to get the most current input line value

// Pay attention that inverted logic is used in Plug-n-Play inputs.
// 0 = Input is closed
// 1 = Input is open
void onInputChange()
{
	// This function returns pointer to current input interface, in our case the PnP_BasicInputInterface assigned to
	// the physical input module.
	// Using that code will allow you to have the same implementation when mulitple physical modules are used for the inputs.
	PnP_InputInterface* pInput = PnP_InputInterface::GetCurrentInterface();

	serial.print("Input ");
	serial.print(pInput->GetUserData());
	serial.print(" changed to: ");
	serial.println(pInput->GetLastValue());
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

	// Initialize serial interface object
	serial.Init();

	// Initialize the inputs module
	inputModule.Init();

	// Use te same onInputChange callback for all the inputs
	for (uint8_t i=0; i<inputModule.numberOfInputs; i++) {
		// User data will be the number of input line for the module
		inputs[i].SetUserData(i);

		// The OnInput callback
		inputs[i].SetOnChange(onInputChange);

		// Assign the interface instance to proper line of the input module
		inputModule.AssignInterface(i, inputs[i]);
	}

	serial.println("Starting...");
}

void loop() {
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
