#ifndef __PNP_MODULE_2INPUTS_H__
#define __PNP_MODULE_2INPUTS_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../Core/PnP_PlugAndPlayDevice.h"
#include "../Core/PnP_PlugAndPlayI2C.h"
#include "../Core/PnP_InputInterface.h"
#include "../Core/PnP_InputInterfaceProvider.h"

class PnP_Module_2Inputs : protected EBF_HalInstance, public PnP_InputInterfaceProvider {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_Module_2Inputs");

	public:
		PnP_Module_2Inputs();

		static const uint8_t numberOfInputs = 2;

		uint8_t Init();

		// Returns current value of the specified input line
		uint8_t GetValue(uint8_t index);
		// Returns current values of all the input lines
		uint8_t GetValues();
		// Returns last value of the specified input line as it appeared while reading from the chip
		uint8_t GetLastValue(uint8_t index);
		// Returns all last values as it appeared while reading from the chip
		uint8_t GetLastValues();

		PnP_InputInterface* GetCurrentInterface();

		// Set callback functions for specific input
		uint8_t SetOnChange(uint8_t index, EBF_CallbackType onChangeCallback);

		// Assign interface instance to specified input index
		uint8_t AssignInterface(uint8_t index, PnP_InputInterface* pIfInstance);
		uint8_t AssignInterface(uint8_t index, PnP_InputInterface& IfInstance) {
			return AssignInterface(index, &IfInstance);
		}

		// Returns input index that caused the callback function call
		// You can have the same callback function for all the inputs events
		// where you can call the GetEventIndex to know which input actually changed
		uint8_t GetEventIndex();

		typedef union {
			struct {
				uint32_t index : 3;		// up to 8 inputs
				uint32_t event : 8;		// input event that should be executed
				uint32_t reserved : 21;
			} fields;
			uint32_t uint32;
		} PostponedInterruptData;

		uint8_t PostponeProcessing(uint8_t eventIndex, uint8_t inputValues);
		uint8_t InInterrupt() {
			EBF_Logic *pLogic = EBF_Logic::GetInstance();
			return pLogic->IsRunFromIsr();
		}

	protected:
		void SetPollingInterval(uint32_t ms);

	private:
		uint8_t Process();
		void ProcessInterrupt();

	 	uint8_t GetIntLine(uint8_t line, uint8_t &value);

		uint8_t GetValue_IIP(uint8_t index) { return this->GetValue(index); }
		uint8_t GetLastValue_IIP(uint8_t index) { return this->GetLastValue(index); }
		unsigned long millis_IIP() { return this->millis(); }
		void SetPollingInterval_IIP(uint32_t ms) { this->SetPollingInterval(ms); }
		uint32_t GetPollingInterval_IIP() { return this->GetPollingInterval(); }

		// The onChangeCallback should be treated as a pointer to an interface instance after assignment
		PnP_InputInterface* GetAsInputInterface(uint8_t index) { return (PnP_InputInterface*)(onChangeCallback[index]); }

	protected:
		PnP_PlugAndPlayI2C *pPnPI2C;

		uint8_t lastValues;
		uint8_t currentEventIndex;

		// Callbacks
		EBF_CallbackType onChangeCallback[numberOfInputs];

		// Interface assigned flag
		uint8_t isInterfaceAssigned;
};

#endif
