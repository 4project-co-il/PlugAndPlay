#ifndef __PNP_DEVICE_INFO_INTERFACE_H__
#define __PNP_DEVICE_INFO_INTERFACE_H__

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

class PnP_DeviceInfoInterface {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_DeviceInfoInterface");

	public:
		PnP_DeviceInfoInterface() {}

	private:
		static const uint8_t eepromI2cAddress = 0x50;
		static const uint8_t eepromPageSize = 16;

	protected:
		uint8_t IsHeaderValid(PnP_DeviceInfo &deviceInfo);
		uint8_t GetDeviceInfo(EBF_I2C* pI2C, PnP_DeviceInfo &deviceInfo, uint8_t routingLevel = 0);
		uint8_t GetDeviceParameters(EBF_I2C* pI2C, uint8_t routingLevel, uint8_t *pParams, uint8_t maxSize);
};

#endif
