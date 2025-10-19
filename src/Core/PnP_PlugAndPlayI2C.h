#ifndef __PNP_PLUGANDPLAY_I2C_H__
#define __PNP_PLUGANDPLAY_I2C_H__

#include <Arduino.h>
#include <Wire.h>

#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Core/EBF_I2C.h"
#include "PnP_PlugAndPlayHub.h"

class PnP_PlugAndPlayI2C : public EBF_I2C {
	public:
		PnP_PlugAndPlayI2C(EBF_I2C &i2c, PnP_PlugAndPlayHub* pHub, uint8_t port);

		void beginTransmission(uint8_t address);

		uint8_t GetPortNumber() { return portNumber; }
		PnP_PlugAndPlayHub* GetHub() { return pHub; }

	private:
		PnP_PlugAndPlayHub* pHub;
		uint8_t portNumber;

};

#endif
