#ifndef __PNP_MODULE_8OUTPUTS_H__
#define __PNP_MODULE_8OUTPUTS_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include <Wire.h>
#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Products/EBF_Module_8Outputs.h"
#include "../Core/PnP_PlugAndPlayDevice.h"
#include "../Core/PnP_PlugAndPlayManager.h"
#include "../Core/PnP_PlugAndPlayI2C.h"
#include "../Core/PnP_OutputInterface.h"
#include "../Core/PnP_OutputInterfaceProvider.h"

class PnP_Module_8Outputs : public EBF_Module_8Outputs, public PnP_OutputInterfaceProvider {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_Module_8Outputs");

	public:
		PnP_Module_8Outputs();

		static const uint8_t numberOfOutputs = 8;

		uint8_t Init();

		// Assign interface instance to specified output index
		uint8_t AssignInterface(uint8_t index, PnP_OutputInterface* pIfInstance);
		uint8_t AssignInterface(uint8_t index, PnP_OutputInterface& IfInstance) {
			return AssignInterface(index, &IfInstance);
		}

	private:
		// Output interface
		PnP_OutputInterface *pInterfaces[numberOfOutputs];

		uint8_t SetValue_OIP(uint8_t index, float value);
		float GetValue_OIP(uint8_t index);
		unsigned long millis_OIP() { return this->millis(); }
		unsigned long micros_OIP() { return this->micros(); }
		void SetPollingInterval_OIP(uint32_t ms) { this->SetPollingInterval(ms); }
		uint32_t GetPollingInterval_OIP() { return this->GetPollingInterval(); }

			// Interface assigned flag
		uint8_t isInterfaceAssigned;

	protected:
		// Override ExecuteCallback and Process to add assigned interfaces processing logic
		void ExecuteCallback();
		uint8_t Process();
		void SetPollingInterval(uint32_t ms);
};

#endif
