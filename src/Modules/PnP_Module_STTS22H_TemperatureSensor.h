#ifndef _PNP_MODULE_STTS22H_TEMPERATURESENSOR_H__
#define _PNP_MODULE_STTS22H_TEMPERATURESENSOR_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include <Wire.h>
#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Products/EBF_STTS22H_TemperatureSensor.h"
#include "../Core/PnP_PlugAndPlayDevice.h"
#include "../Core/PnP_PlugAndPlayManager.h"
#include "../Core/PnP_PlugAndPlayI2C.h"

class PnP_Module_STTS22H_TemperatureSensor : public EBF_STTS22H_TemperatureSensor {
	public:
		PnP_Module_STTS22H_TemperatureSensor();

		uint8_t Init();

};

#endif
