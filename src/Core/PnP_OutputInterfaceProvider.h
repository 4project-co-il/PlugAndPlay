#ifndef __PNP_OUTPUT_INTERFACE_PROVIDER_H__
#define __PNP_OUTPUT_INTERFACE_PROVIDER_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "PnP_OutputInterface.h"

class PnP_OutputInterface;

class PnP_OutputInterfaceProvider {
	public:
		friend class PnP_OutputInterface;
		friend class PnP_SimpleLedInterface;
		friend class PnP_DirectPwmInterface;

		// Assign interface instance to output provider
		virtual uint8_t AssignInterface(uint8_t index, PnP_OutputInterface* pIfInstance) = 0;

	private:
		// Set current value of the output
		virtual uint8_t SetValue_OIP(uint8_t index, float value) = 0;

		// Get current value of the output
		virtual float GetValue_OIP(uint8_t index) = 0;

		// Get output update frequency
		virtual uint16_t GetUpdateFrequency(uint8_t index) = 0;

		// Access to current millis value of the HAL instance via OutputInterfaceProvider
		virtual unsigned long millis_OIP() = 0;
		// Access to current micros value of the HAL instance via OutputInterfaceProvider
		virtual unsigned long micros_OIP() = 0;

		// Access to polling interval of the HAL instance via OutputInterfaceProvider
		virtual void SetPollingInterval_OIP(uint32_t ms) = 0;

		// Access to polling interval of the HAL instance via OutputInterfaceProvider
		virtual uint32_t GetPollingInterval_OIP() = 0;

};

#endif
