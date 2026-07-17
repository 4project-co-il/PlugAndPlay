#ifndef __PNP_MODULE_1SIMPLELED_H__
#define __PNP_MODULE_1SIMPLELED_H__

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

class PnP_Module_1SimpleLed : protected EBF_HalInstance {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_Module_1SimpleLed");

	public:
		PnP_Module_1SimpleLed();

		uint8_t Init();

		uint8_t On();
		uint8_t Off();

	private:
		uint8_t Process();

	 	uint8_t SetIntLine(uint8_t line, uint8_t value);

	private:
		PnP_PlugAndPlayI2C *pPnPI2C;
};

#endif
