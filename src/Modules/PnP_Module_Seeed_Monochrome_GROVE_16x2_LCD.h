#ifndef _PNP_MODULE_SEEED_MONOCHROME_GROVE_16x2_LCD_H__
#define _PNP_MODULE_SEEED_MONOCHROME_GROVE_16x2_LCD_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include <Wire.h>
#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Products/EBF_Seeed_Monochrome_GROVE_16x2_LCD.h"
#include "../Core/PnP_PlugAndPlayDevice.h"
#include "../Core/PnP_PlugAndPlayManager.h"
#include "../Core/PnP_PlugAndPlayI2C.h"

class PnP_Module_Seeed_Monochrome_GROVE_16x2_LCD : public EBF_Seeed_Monochrome_GROVE_16x2_LCD {
	public:
		PnP_Module_Seeed_Monochrome_GROVE_16x2_LCD();

		uint8_t Init();

};

#endif
