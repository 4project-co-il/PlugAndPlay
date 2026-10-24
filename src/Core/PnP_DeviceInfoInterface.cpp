#include "PnP_DeviceInfoInterface.h"


uint8_t PnP_DeviceInfoInterface::GetDeviceInfo(EBF_I2C* pI2C, PnP_DeviceInfo &deviceInfo, uint8_t routingLevel)
{
	uint8_t rc;

	// Read the device info
	pI2C->beginTransmission(eepromI2cAddress + routingLevel);
	// Device info resides at the beginning of the EEPROM
	pI2C->write(0);
	rc = pI2C->endTransmission(false);
	if (rc != 0) {
		EBF_REPORT_AND_RETURN(EBF_COMMUNICATION_PROBLEM);
	}

	pI2C->requestFrom(eepromI2cAddress + routingLevel, sizeof(PnP_DeviceInfo));

	rc = pI2C->readBytes((uint8_t*)&deviceInfo, sizeof(PnP_DeviceInfo));
	// Strange, shuuld not happen with PnP device, skip it
	if (rc != sizeof(PnP_DeviceInfo)) {
		EBF_REPORT_AND_RETURN(EBF_COMMUNICATION_PROBLEM);
	}

	// This is not a PnP device, skip it
	if (!IsHeaderValid(deviceInfo)) {
		EBF_REPORT_AND_RETURN(EBF_COMMUNICATION_PROBLEM);
	}

	EBF_REPORT_AND_RETURN(EBF_OK);
}

uint8_t PnP_DeviceInfoInterface::GetDeviceParameters(EBF_I2C* pI2C, uint8_t routingLevel, uint8_t *pParams, uint8_t maxSize)
{
	uint8_t rc;

	// No parameters should be read
	if (maxSize == 0) {
		EBF_REPORT_AND_RETURN(EBF_OK);
	}

	// Read the device parameters
	pI2C->beginTransmission(eepromI2cAddress + routingLevel);
	// Parameters are right after the device info structure
	pI2C->write(sizeof(PnP_DeviceInfo));
	rc = pI2C->endTransmission(false);
	if (rc != 0) {
		EBF_REPORT_AND_RETURN(EBF_COMMUNICATION_PROBLEM);
	}

	pI2C->requestFrom(eepromI2cAddress + routingLevel, maxSize);

	rc = pI2C->readBytes(pParams, maxSize);
	// Strange, should not happen with PnP device, skip it
	if (rc != maxSize) {
		EBF_REPORT_AND_RETURN(EBF_COMMUNICATION_PROBLEM);
	}

	EBF_REPORT_AND_RETURN(EBF_OK);
}

uint8_t PnP_DeviceInfoInterface::IsHeaderValid(PnP_DeviceInfo &deviceInfo)
{
	// "PnP*" = 0x506E502A
	if (deviceInfo.headerId == 0x506E502A) {
		return 1;
	} else {
		return 0;
	}
}
