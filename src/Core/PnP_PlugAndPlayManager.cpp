#include <Arduino.h>
#include <Wire.h>
#include "PnP_PlugAndPlayManager.h"
#include "PnP_PlugAndPlayHub.h"
#include "PnP_PlugAndPlayI2C.h"

#ifdef PNP_DEBUG_ENUMERATION
#include "PnP_Serial.h"
extern PnP_Serial serial;
#endif

// PnP_PlugAndPlay implementation
PnP_PlugAndPlayManager *PnP_PlugAndPlayManager::pStaticInstance = NULL;

PnP_PlugAndPlayManager::PnP_PlugAndPlayManager()
{
	memset(pnpI2CArr, 0, sizeof(EBF_I2C*) * PNP_NUMBER_OF_I2C_INTERFACES);
}

PnP_PlugAndPlayManager *PnP_PlugAndPlayManager::GetInstance()
{
	if (pStaticInstance == NULL) {
		pStaticInstance = new PnP_PlugAndPlayManager();
		pStaticInstance->Init();
	}

	return pStaticInstance;
}

uint8_t PnP_PlugAndPlayManager::Init()
{
	uint8_t rc;
	PnP_DeviceInfo deviceInfo;
	uint8_t interruptMapping[PnP_PlugAndPlayHub::maxPorts * 2] = { (uint8_t)(-1) };	// 2 interrupts per port, up to 8 ports

#ifdef PNP_DEBUG_ENUMERATION
	serial.println(F("PnP Init"));
#endif

	// I2C interfaces initialization.
	// Controller might have different PnP ports connected to different physical I2C interfaces
	// Number of the interfaces and ports mapping arrive from the variants.h file of Arduino board definition
	for (uint8_t i=0; i<PNP_NUMBER_OF_I2C_INTERFACES; i++) {
		switch (i)
		{
		case 0:
			pnpI2CArr[i] = new EBF_I2C(Wire);
			break;
#if PNP_NUMBER_OF_I2C_INTERFACES > 1
		case 1:
			pnpI2CArr[i] = new EBF_I2C(Wire1);
			break;
#endif
#if PNP_NUMBER_OF_I2C_INTERFACES > 2
		case 2:
			pnpI2CArr[i] = new EBF_I2C(Wire2);
			break;
#endif
#if PNP_NUMBER_OF_I2C_INTERFACES > 3
		case 3:
			pnpI2CArr[i] = new EBF_I2C(Wire3);
			break;
#endif
		}

		if (pnpI2CArr[i] == NULL) {
			EBF_REPORT_ERROR(EBF_NOT_ENOUGH_MEMORY);
			return EBF_NOT_ENOUGH_MEMORY;
		}

		pnpI2CArr[i]->Init();
		pnpI2CArr[i]->SetClock(400000);
	}

	pMainHub = new PnP_PlugAndPlayHub();
	if (pMainHub == NULL) {
		EBF_REPORT_ERROR(EBF_NOT_ENOUGH_MEMORY);
		return EBF_NOT_ENOUGH_MEMORY;
	}

	// Read the configuration of the main hub
	// Devices are considered as level 0, main hub - level 3, extender hubs - levels 4,5,6,7
	// Controller I2C interface is specified in variants.h
	rc = GetDeviceInfo(pnpI2CArr[PNP_CONTROLLER_INTERNAL_I2C_INTERFACE_INDEX], deviceInfo, PNP_EEPROM_MAIN_HUB);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(EBF_COMMUNICATION_PROBLEM);
		return EBF_COMMUNICATION_PROBLEM;
	}

	// There are parameters, for embedded HUBs those are interrupt mappings
	if (deviceInfo.paramsLength != 0) {
		rc = GetDeviceParameters(pnpI2CArr[PNP_CONTROLLER_INTERNAL_I2C_INTERFACE_INDEX], PNP_EEPROM_MAIN_HUB, interruptMapping, min(deviceInfo.paramsLength, sizeof(interruptMapping)));
		if (rc != EBF_OK) {
			EBF_REPORT_ERROR(EBF_COMMUNICATION_PROBLEM);
			return EBF_COMMUNICATION_PROBLEM;
		}
	}

#ifdef PNP_DEBUG_ENUMERATION
	serial.println(F("PnP Main HUB info:"));
	PrintDeviceInfo(deviceInfo, interruptMapping);
#endif

	rc = pMainHub->Init(NULL, 0, deviceInfo, interruptMapping);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	rc = InitHubs(pMainHub);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

uint8_t PnP_PlugAndPlayManager::InitHubs(PnP_PlugAndPlayHub *pHub)
{
	uint8_t rc;
	EBF_I2C *pI2C;
	PnP_DeviceInfo deviceInfo;
	uint8_t parameters[32];

#ifdef PNP_DEBUG_ENUMERATION
	serial.println(F("PnP InitHubs"));
#endif

	// Loop over all the ports of that HUB
	for (uint8_t port=0; port<pHub->numberOfPorts; port++) {
#ifdef PNP_DEBUG_ENUMERATION
	serial.print(F("Checking port: "));
	serial.println(port);
#endif
		// I2C interface for current port
		pI2C = pnpI2CArr[PnP_GetInterfaceIndexForPort(port)];

		rc = pHub->SwitchToPort(pI2C, port);
		if (rc != EBF_OK) {
			// Something is wrong...
			EBF_REPORT_ERROR(rc);
			return rc;
		}

		// Read a regular PnP device info
		rc = GetDeviceInfo(pI2C, deviceInfo, PNP_EEPROM_DEVICE);
		if (rc != EBF_OK) {
			// There is no PnP device connected to that port, check maybe there's another HUB set up for the next routing level
			if (pHub->routingLevel + 1 > maxRoutingLevels) {
				// We're reached max routing levels
				continue;
			}

#ifdef PNP_DEBUG_ENUMERATION
			serial.print(F("Nothing connected, try next level: "));
			serial.println(pHub->routingLevel+1);
#endif

			// Try to get device for the next routing level
			// Main HUB will be level 1, generic HUBs will be levels 4,5,6,7
			rc = GetDeviceInfo(pI2C, deviceInfo, pHub->routingLevel + 1);
			if (rc != EBF_OK) {
#ifdef PNP_DEBUG_ENUMERATION
				serial.println(F("Nothing on next level as well"));
#endif
					// No HUB either, mark it with (-1), so it will be skipped in searches
				pHub->pPortInfo[port].numberOfEndpoints = (uint8_t)(-1);
				continue;
			}
		}

#ifdef PNP_DEBUG_ENUMERATION
		serial.println(F("Found device:"));
		PrintDeviceInfo(deviceInfo);
#endif

		// In case it's a HUB, create its new instance and connect all the pointers
		if (deviceInfo.deviceIDs[0] == PnP_DeviceId::PNP_ID_EXTENDER_HUB) {
			// Read the parameters in case the configuration says so
			if (deviceInfo.paramsLength > 0) {
				rc = GetDeviceParameters(pI2C, pHub->routingLevel + 1, &parameters[0], min(deviceInfo.paramsLength, sizeof(parameters)));

				if (rc != EBF_OK) {
					// Something is wrong... should not happen
					EBF_REPORT_ERROR(rc);
					return rc;
				}
			}

#ifdef PNP_DEBUG_ENUMERATION
			serial.println(F("Extender HUB found"));
			PrintDeviceInfo(deviceInfo, parameters);
#endif

			PnP_PlugAndPlayHub* pNewHub = new PnP_PlugAndPlayHub();

			rc = pNewHub->Init(pHub, port, deviceInfo, &parameters[0]);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}

			// Save the pointer
			pHub->pPortInfo[port].numberOfEndpoints = 1;
			pHub->pPortInfo[port].pConnectedInstanes = (EBF_HalInstance**)malloc(sizeof(EBF_HalInstance*) * pHub->pPortInfo[port].numberOfEndpoints);
			pHub->pPortInfo[port].pConnectedInstanes[0] = pNewHub;

			// Initialize the new HUB connections
			rc = InitHubs(pNewHub);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}
		}
	}

	return EBF_OK;
}

uint8_t PnP_PlugAndPlayManager::IsHeaderValid(PnP_DeviceInfo &deviceInfo)
{
	// "PnP*" = 0x506E502A
	if (deviceInfo.headerId == 0x506E502A) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t PnP_PlugAndPlayManager::GetDeviceInfo(EBF_I2C* pPnpI2C, PnP_DeviceInfo &deviceInfo, uint8_t routingLevel)
{
	uint8_t rc;

	// Read the device info
	pPnpI2C->beginTransmission(eepromI2cAddress + routingLevel);
	// Device info resides at the beginning of the EEPROM
	pPnpI2C->write(0);
	rc = pPnpI2C->endTransmission(false);
	if (rc != 0) {
		return EBF_COMMUNICATION_PROBLEM;
	}

	pPnpI2C->requestFrom(eepromI2cAddress + routingLevel, sizeof(PnP_DeviceInfo));

	rc = pPnpI2C->readBytes((uint8_t*)&deviceInfo, sizeof(PnP_DeviceInfo));
	// Strange, shuuld not happen with PnP device, skip it
	if (rc != sizeof(PnP_DeviceInfo)) {
		return EBF_COMMUNICATION_PROBLEM;
	}

	// This is not a PnP device, skip it
	if (!IsHeaderValid(deviceInfo)) {
		return EBF_COMMUNICATION_PROBLEM;
	}

	return EBF_OK;
}

uint8_t PnP_PlugAndPlayManager::GetDeviceParameters(EBF_I2C* pPnpI2C, uint8_t routingLevel, uint8_t *pParams, uint8_t maxSize)
{
	uint8_t rc;

	// No parameters should be read
	if (maxSize == 0) {
		return EBF_OK;
	}

	// Read the device parameters
	pPnpI2C->beginTransmission(eepromI2cAddress + routingLevel);
	// Parameters are right after the device info structure
	pPnpI2C->write(sizeof(PnP_DeviceInfo));
	rc = pPnpI2C->endTransmission(false);
	if (rc != 0) {
		EBF_REPORT_ERROR(EBF_COMMUNICATION_PROBLEM);
		return EBF_COMMUNICATION_PROBLEM;
	}

	pPnpI2C->requestFrom(eepromI2cAddress + routingLevel, maxSize);

	rc = pPnpI2C->readBytes(pParams, maxSize);
	// Strange, should not happen with PnP device, skip it
	if (rc != maxSize) {
		EBF_REPORT_ERROR(EBF_COMMUNICATION_PROBLEM);
		return EBF_COMMUNICATION_PROBLEM;
	}

	return EBF_OK;
}

uint8_t PnP_PlugAndPlayManager::AssignDevice(
	EBF_HalInstance* pHalInstance,
	PnP_DeviceInfo& deviceInfo,
	uint8_t& endpointIndex,
	PnP_PlugAndPlayI2C** pI2CRouter,
	PnP_PlugAndPlayHub** pAssignedHub,
	PnP_PlugAndPlayHub* pHub)
{
	uint8_t rc;
	EBF_I2C *pI2C;
	PnP_PlugAndPlayHub::PortInfo *pPortInfo;

	if (pHub == NULL) {
		pHub = pMainHub;
	}

	// Check all the ports
	for (uint8_t port=0; port<pHub->numberOfPorts; port++) {
		pPortInfo = &pHub->pPortInfo[port];

		// Nothing is connected to that port

		if (pPortInfo->numberOfEndpoints == (uint8_t)(-1)) {
			continue;
		}

		// I2C interface for current port
		pI2C = pnpI2CArr[PnP_GetInterfaceIndexForPort(port)];

		rc = pHub->SwitchToPort(pI2C, port);
		if (rc != EBF_OK) {
			// Something is wrong
			EBF_REPORT_ERROR(rc);
			return rc;
		}

		// If the connected device is a HUB, search there
		if (pPortInfo->numberOfEndpoints > 0 &&
			pPortInfo->pConnectedInstanes[0]->GetId() == PnP_DeviceId::PNP_ID_EXTENDER_HUB) {
				rc = AssignDevice(pHalInstance, deviceInfo, endpointIndex, pI2CRouter, pAssignedHub, (PnP_PlugAndPlayHub*)(pPortInfo->pConnectedInstanes[0]));
				if (rc == EBF_OK) {
					// Device was found by inner HUB instance
					return EBF_OK;
				}
		}

		// Check if current port is connected to the needed device
		rc = GetDeviceInfo(pI2C, deviceInfo);
		if (rc != EBF_OK) {
			// Should not happen since we already communicated with that device before...
			continue;
		}

		// Check all endpoints
		for (endpointIndex=0; endpointIndex<deviceInfo.numberOfEndpoints; endpointIndex++) {
			// Skip endpoints that were already assigned to a HAL instance
			if (pPortInfo->numberOfEndpoints > 0 && pPortInfo->pConnectedInstanes[endpointIndex] != NULL) {
				continue;
			}

			if (deviceInfo.deviceIDs[endpointIndex] == pHalInstance->GetId()) {
				// Found the matching endpoint
				break;
			} else {
				// Not the needed device
				continue;
			}
		}

		// No mathcing endpoints found in that device
		if (endpointIndex == deviceInfo.numberOfEndpoints) {
			continue;
		}

		// We found the needed device
		// Store it's pointer in the HUB and create PnP I2C linkage that can route to the needed port
		if (pPortInfo->numberOfEndpoints == 0) {
			// array of connected instances wasn't initiates yet
			pPortInfo->numberOfEndpoints = deviceInfo.numberOfEndpoints;
			pPortInfo->pConnectedInstanes = (EBF_HalInstance**)malloc(sizeof(EBF_HalInstance*) * pPortInfo->numberOfEndpoints);
		}

		pPortInfo->pConnectedInstanes[endpointIndex] = pHalInstance;
		*pI2CRouter = new PnP_PlugAndPlayI2C(*(pI2C), pHub, port);
		*pAssignedHub = pHub;

		if (*pI2CRouter == NULL) {
			EBF_REPORT_ERROR(EBF_NOT_ENOUGH_MEMORY);
			return EBF_NOT_ENOUGH_MEMORY;
		}

		return EBF_OK;
	}

	EBF_REPORT_ERROR(EBF_NOT_INITIALIZED);
	return EBF_NOT_INITIALIZED;
}

uint8_t PnP_PlugAndPlayManager::WriteDeviceEEPROM(uint8_t i2cAddress, PnP_DeviceInfo &deviceInfo, uint8_t* pParams, uint8_t paramsSize)
{
	uint8_t rc;

	PnP_PlugAndPlayManager *pInstance = PnP_PlugAndPlayManager::GetInstance();
	uint8_t* pData = (uint8_t*)&deviceInfo;

	// Write page-by-page
	uint16_t size = sizeof(PnP_DeviceInfo);
	uint16_t offsset = 0;
	while (size > 0) {
		if (size >= eepromPageSize) {
			// Full page can be written
			rc = pInstance->WriteDeviceEepromPage(i2cAddress, offsset, pData+offsset, eepromPageSize);

			offsset += eepromPageSize;
			size -= eepromPageSize;
		} else {
			// Less than a page left
			rc = pInstance->WriteDeviceEepromPage(i2cAddress, offsset, pData+offsset, size);

			offsset += size;
			size  = 0;
		}

		if (rc != EBF_OK) {
			return rc;
		}
	}

	size = paramsSize;
	offsset = 0;		// Parameters are started right after the device info
	while (size > 0) {
		if (size >= eepromPageSize) {
			// Full page can be written
			rc = pInstance->WriteDeviceEepromPage(i2cAddress, offsset + sizeof(PnP_DeviceInfo), pParams+offsset, eepromPageSize);

			offsset += eepromPageSize;
			size -= eepromPageSize;
		} else {
			// Less than a page left
			rc = pInstance->WriteDeviceEepromPage(i2cAddress, offsset + sizeof(PnP_DeviceInfo), pParams+offsset, size);

			offsset += size;
			size = 0;
		}

		if (rc != EBF_OK) {
			return rc;
		}
	}

	return EBF_OK;
}

uint8_t PnP_PlugAndPlayManager::WriteDeviceEepromPage(uint8_t i2cAddress, uint8_t eepromAddress, uint8_t* pData, uint8_t size)
{
	uint8_t rc;

	EBF_I2C *pPnpI2C = pnpI2CArr[PNP_EEPROM_PROGRAMMING_INTERFACE_INDEX];

	// Main HUB don't need any switches, any other device should be connected to port 0 to write its EEPROM
	if (i2cAddress != 0x50 + PnP_PlugAndPlayManager::PNP_EEPROM_MAIN_HUB) {
		rc = pMainHub->SwitchToPort(pPnpI2C, 0);
		if (rc != EBF_OK) {
			EBF_REPORT_ERROR(rc);
			return rc;
		}
	}

	pPnpI2C->beginTransmission(i2cAddress);
	pPnpI2C->write(eepromAddress);
	pPnpI2C->write(pData, size);

	rc = pPnpI2C->endTransmission(true);
	if (rc != 0) {
		EBF_REPORT_ERROR(EBF_COMMUNICATION_PROBLEM);
		return EBF_COMMUNICATION_PROBLEM;
	}

	// Add delay after each page. max write time is 5ms
	delay(5);

	return EBF_OK;
}

#ifdef PNP_DEBUG_ENUMERATION
void PnP_PlugAndPlayManager::PrintDeviceInfo(PnP_DeviceInfo &deviceInfo, uint8_t* pParams)
{
	uint8_t i;

	serial.print(F("  headerId: 0x"));
	serial.println(deviceInfo.headerId, HEX);

	serial.print(F("  version: "));
	serial.println(deviceInfo.version);

	serial.print(F("  numberOfPorts: "));
	serial.println(deviceInfo.numberOfPorts);

	serial.print(F("  numberOfEndpoints: "));
	serial.println(deviceInfo.numberOfEndpoints);

	serial.print(F("  numberOfInterrupts: "));
	serial.println(deviceInfo.numberOfInterrupts);

	serial.print(F("  interrupt1Mode: "));
	serial.println(deviceInfo.interrupt1Mode);

	serial.print(F("  interrupt2Mode: "));
	serial.println(deviceInfo.interrupt2Mode);

	serial.print(F("  interrupt1Endpoint: "));
	serial.println(deviceInfo.interrupt1Endpoint);

	serial.print(F("  interrupt2Endpoint: "));
	serial.println(deviceInfo.interrupt2Endpoint);

	serial.print(F("  paramsLength: "));
	serial.println(deviceInfo.paramsLength);

	for (i=0; i<deviceInfo.numberOfEndpoints; i++) {
		serial.print(F("  EndPoint["));
		serial.print(i);
		serial.println(F("]: "));

		serial.print(F("    ID: "));
		serial.println(deviceInfo.deviceIDs[i]);

		serial.print(F("    index: "));
		serial.println(deviceInfo.endpointData[i].endpointId);

		serial.print(F("    I2C address: 0x"));
		serial.println(deviceInfo.endpointData[i].i2cAddress, HEX);
	}

	for (i=0; i<deviceInfo.paramsLength; i++) {
		serial.print(F("  Params["));
		serial.print(i);
		serial.print(F("]: "));
		serial.println(pParams[i]);
	}
}
#endif
