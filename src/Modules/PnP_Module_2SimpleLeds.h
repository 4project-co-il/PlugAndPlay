#ifndef __PNP_MODULE_2SIMPLELEDS_H__
#define __PNP_MODULE_2SIMPLELEDS_H__

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
#include "../Core/PnP_OutputInterface.h"
#include "../Core/PnP_OutputInterfaceProvider.h"

class PnP_Module_2SimpleLeds : protected EBF_HalInstance, public PnP_OutputInterfaceProvider {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_Module_2SimpleLeds");

	public:
		PnP_Module_2SimpleLeds();

		static const uint8_t numberOfOutputs = 2;

		uint8_t Init();

		uint8_t On(uint8_t index);
		uint8_t Off(uint8_t index);

		// Set value of specified index
		uint8_t SetValue(uint8_t index, uint8_t value);
		// Set values of both lines with one call
		uint8_t SetValues(uint8_t values);
		// Gets current led value for specified index
		uint8_t GetValue(uint8_t index);

		// Assign interface instance
		uint8_t AssignInterface(uint8_t index, PnP_OutputInterface* pIfInstance);
		uint8_t AssignInterface(uint8_t index, PnP_OutputInterface& IfInstance) {
			return AssignInterface(index, &IfInstance);
		}

	private:
		void SetPollingInterval(uint32_t ms);
		uint8_t Process();

		uint8_t SetIntLine(uint8_t line, uint8_t value);
	 	uint8_t GetIntLine(uint8_t line, uint8_t &value);

	private:
		// Output interface
		PnP_OutputInterface *pInterfaces[numberOfOutputs];

		uint8_t SetValue_OIP(uint8_t index, float value);
		float GetValue_OIP(uint8_t index);
		unsigned long millis_OIP() { return this->millis(); }
		unsigned long micros_OIP() { return this->micros(); }
		void SetPollingInterval_OIP(uint32_t ms);
		uint32_t GetPollingInterval_OIP() { return this->GetPollingInterval(); }

	private:
		PnP_PlugAndPlayI2C *pPnPI2C;
};

#endif
