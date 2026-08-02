#ifndef __PNP_MODULE_8INPUTS_H__
#define __PNP_MODULE_8INPUTS_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include <Wire.h>
#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Products/EBF_Module_8Inputs.h"
#include "../Core/PnP_PlugAndPlayDevice.h"
#include "../Core/PnP_PlugAndPlayManager.h"
#include "../Core/PnP_PlugAndPlayI2C.h"
#include "../Core/PnP_InputInterface.h"
#include "../Core/PnP_InputInterfaceProvider.h"

class PnP_Module_8Inputs : public EBF_Module_8Inputs, public PnP_InputInterfaceProvider {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_Module_8Inputs");

	public:
		PnP_Module_8Inputs();

		uint8_t Init();

		// Returns current value of the specified input line
		uint8_t GetValue(uint8_t index) { return EBF_Module_8Inputs::GetValue(index); }
		// Returns current values of all the input lines
		uint8_t GetValues() { return EBF_Module_8Inputs::GetValues(); }
		// Returns last value of the specified input line as it appeared while reading from the chip
		uint8_t GetLastValue(uint8_t index) { return EBF_Module_8Inputs::GetLastValue(index); }
		// Returns all last values as it appeared while reading from the chip
		uint8_t GetLastValues() { return EBF_Module_8Inputs::GetLastValues(); }

		PnP_InputInterface* GetCurrentInterface();

		// Assign interface instance to specified input index
		uint8_t AssignInterface(uint8_t index, PnP_InputInterface* pIfInstance);
		uint8_t AssignInterface(uint8_t index, PnP_InputInterface& IfInstance) {
			return AssignInterface(index, &IfInstance);
		}

	private:
		uint8_t GetValue_IIP(uint8_t index) { return this->GetValue(index); }
		uint8_t GetLastValue_IIP(uint8_t index) { return this->GetLastValue(index); }
		unsigned long millis_IIP() { return this->millis(); }
		void SetPollingInterval_IIP(uint32_t ms) { this->SetPollingInterval(ms); }
		uint32_t GetPollingInterval_IIP() { return this->GetPollingInterval(); }

		// The onChangeCallback should be treated as a pointer to an interface instance after assignment
		PnP_InputInterface* GetAsInputInterface(uint8_t index) { return (PnP_InputInterface*)(onChangeCallback[index]); }

		// Interface assigned flag
		uint8_t isInterfaceAssigned;

	protected:
		// Override ExecuteCallback and Process to add assigned interfaces processing logic
		void ExecuteCallback();
		uint8_t Process();
};

#endif
