#ifndef __PNP_INT_LINE_INTERFACE_H__
#define __PNP_INT_LINE_INTERFACE_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include <Wire.h>
#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../Core/PnP_PlugAndPlayDevice.h"
#include "../Core/PnP_PlugAndPlayManager.h"
#include "../Core/PnP_PlugAndPlayI2C.h"

class PnP_IntLineInterface {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_IntLineInterface");

	public:
		PnP_IntLineInterface() {}

	protected:
		uint8_t SetIntLine(PnP_PlugAndPlayI2C* pPnPI2C, uint8_t line, uint8_t value);
	 	uint8_t GetIntLine(PnP_PlugAndPlayI2C* pPnPI2C, uint8_t line, uint8_t &value);

};

#endif
