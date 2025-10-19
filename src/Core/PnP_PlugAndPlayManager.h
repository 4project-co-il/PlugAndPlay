#ifndef __PNP_PLUGANDPLAY_MANAGER_H__
#define __PNP_PLUGANDPLAY_MANAGER_H__

#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Core/EBF_I2C.h"
#include "PnP_PlugAndPlayDevice.h"
#include "PnP_PlugAndPlayHub.h"

class PnP_PlugAndPlayHub;
class PnP_PlugAndPlayI2C;

class PnP_PlugAndPlayManager {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_PlugAndPlayManager");

	public:
		static const uint8_t maxRoutingLevels = 7;
		static const uint8_t maxEndpoints = 8;

		PnP_PlugAndPlayManager();

		uint8_t Init();

		static PnP_PlugAndPlayManager *GetInstance();

		uint8_t AssignDevice(
			EBF_HalInstance* pHalInstance,
			PnP_DeviceInfo& deviceInfo,
			uint8_t& endpointIndex,
			PnP_PlugAndPlayI2C** pI2CRouter,
			PnP_PlugAndPlayHub** pAssignedHub,
			PnP_PlugAndPlayHub* pHub = NULL);

		static uint8_t WriteDeviceEEPROM(uint8_t i2cAddress, PnP_DeviceInfo &deviceInfo, uint8_t* pParams = NULL, uint8_t paramsSize = 0);

	private:
		PnP_PlugAndPlayHub* pMainHub;
		uint8_t IsHeaderValid(PnP_DeviceInfo &deviceInfo);
		uint8_t GetDeviceInfo(EBF_I2C* pPnpI2C, PnP_DeviceInfo &deviceInfo, uint8_t routingLevel = 0);
		uint8_t GetDeviceParameters(EBF_I2C* pPnpI2C, uint8_t routingLevel, uint8_t *pParams, uint8_t maxSize);

		uint8_t InitHubs(PnP_PlugAndPlayHub *pHub);

		uint8_t WriteDeviceEepromPage(uint8_t i2cAddress, uint8_t eepromAddress, uint8_t* pData, uint8_t size);

#ifdef PNP_DEBUG_ENUMERATION
		void PrintDeviceInfo(PnP_DeviceInfo &deviceInfo, uint8_t* pParams=NULL);
#endif

	private:
		static PnP_PlugAndPlayManager* pStaticInstance;

		static const uint8_t eepromI2cAddress = 0x50;
		static const uint8_t eepromPageSize = 16;

		// I2C interfaces array. The number comes from variants.h of Arduino board definition
		EBF_I2C* pnpI2CArr[PNP_NUMBER_OF_I2C_INTERFACES];

	public:
		enum PnP_PnPEepromAddress {
			PNP_EEPROM_DEVICE = 0,			// Devices are always 0
			PNP_EEPROM_MAIN_HUB = 3,
			PNP_EEPROM_EXTENDER_HUB = 4,	// Extenders are 4,5,6,7
		};
};

#endif
