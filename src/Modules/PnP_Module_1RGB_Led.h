#ifndef __PNP_MODULE_1RGB_LED_H__
#define __PNP_MODULE_1RGB_LED_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include <Wire.h>
#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Products/EBF_Module_1RGB_Led.h"
#include "../Core/PnP_PlugAndPlayDevice.h"
#include "../Core/PnP_PlugAndPlayManager.h"
#include "../Core/PnP_PlugAndPlayI2C.h"

class PnP_Module_1RGB_Led : public EBF_Module_1RGB_Led {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_Module_1RGB_Led");

	public:
		PnP_Module_1RGB_Led();

		uint8_t Init();

		uint8_t PowerDown();
		uint8_t PowerUp();

	private:
		uint8_t SetIntLine(uint8_t line, uint8_t value);
};

#endif
