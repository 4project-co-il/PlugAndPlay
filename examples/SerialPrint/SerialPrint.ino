#include <Arduino.h>
#include "PlugAndPlay.h"

// This is an example for basic Plug-n-Play code strucure
// This example creates a 1Hz timer, which will print a counter to serial USB interface.

// Objects creation
// EBF - Event Based Framework is the engine behind the Plug-n-Play processing
EBF_Core EBF;

// Timer object
EBF_Timer printTimer;

// Serial interface via the USB connection. Use IDE Serial Monitor to see the printouts.
// You can use PnP_UART class instead if you want to communicate via UART interface of the development board.
PnP_Serial serial;

int counter = 0;

// This function will be called every time the PRINT_TIMER expires
void onTimer()
{
	// Printing is done the same way as you would do with the Arduino's Serial object
	serial.print("Counter: ");
	serial.println(counter);

	counter++;

	// Timers are one-shot by default. Start it again for next print
	printTimer.Start();
}

// All the setup should be done in that function
void setup()
{
	// Uncomment that line if you want to wait for establishment of the serial connection
	//while (!serial) {}

	// EBF error reporting interface should be set before the initialization
	EBF.SetErrorHandlerSerial(serial);

	// EBF is the first thing that should be initialized, with the maximum timers to be used
	// The parameter specifies how many interrupt events can be queued in transition
	// between the interrupt execution (ISR) and the normal run loop.
	// For light applications without fast events and long execution, number 4 might be enough.
	// For applications with fast burst of events, or long execution for some of them, might need a larger number
	EBF.Init(16);

	// Initialize serial interface object
	serial.Init();

	// Initialize the PRINT_TIMER to 1000mSec (1 second)
	// The onTimer function will be called every time the PRINT_TIMER expires
	printTimer.Init(onTimer, 1000);
	// Start the timer
	printTimer.Start();

	serial.println("Starting...");
}

void loop() {
	// Let EBF to do all the processing
	// Your logic should be done in the callback functions
	EBF.Process();
}
