#ifndef __PNP_MODULE_POWER_CONVERTER_H__
#define __PNP_MODULE_POWER_CONVERTER_H__

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
#include "../Core/PnP_IntLineInterface.h"

class PnP_Module_PowerConverter : protected EBF_HalInstance, protected PnP_IntLineInterface {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_Module_PowerConverter");

	public:
		PnP_Module_PowerConverter();

		uint8_t Init(uint8_t enable = 1);

		// Enables the power converter module
		uint8_t Enable();
		// Disables the power converter module
		uint8_t Disable();
		// Return 1 when power converter reports that output voltage is good, 0 otherwise
		uint8_t IsPowerGood();
		// Returns 1 when power converter is enabled, 0 otherwise
		uint8_t IsEnabled();

	private:
		uint8_t Process() { return EBF_OK; }

	private:
		PnP_PlugAndPlayI2C *pPnPI2C;
};

#endif
