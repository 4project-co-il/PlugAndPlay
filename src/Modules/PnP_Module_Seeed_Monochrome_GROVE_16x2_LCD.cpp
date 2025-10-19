#include "PnP_Module_Seeed_Monochrome_GROVE_16x2_LCD.h"

PnP_Module_Seeed_Monochrome_GROVE_16x2_LCD::PnP_Module_Seeed_Monochrome_GROVE_16x2_LCD() : EBF_Seeed_Monochrome_GROVE_16x2_LCD(NULL)
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_SEEED_MONOCHROME_GROVE_16x2_LCD;
}

uint8_t PnP_Module_Seeed_Monochrome_GROVE_16x2_LCD::Init()
{
	uint8_t rc = EBF_OK;
	PnP_DeviceInfo deviceInfo;
	uint8_t endpointIndex;
	PnP_PlugAndPlayI2C *pPnPI2C;
	PnP_PlugAndPlayHub *pAssignedHub;

	PnP_PlugAndPlayManager *pPnpManager = PnP_PlugAndPlayManager::GetInstance();

	// Assign the current instance to physical PnP device and get all needed information
	rc = pPnpManager->AssignDevice(this, deviceInfo, endpointIndex, &pPnPI2C, &pAssignedHub);
	if(rc != EBF_OK) {
		return rc;
	}

	pI2C = pPnPI2C;

	// Initialize the device
	rc = EBF_Seeed_Monochrome_GROVE_16x2_LCD::Init(deviceInfo.endpointData[endpointIndex].i2cAddress);
	if (rc != EBF_OK) {
		return rc;
	}

	// Fix type and ID after the EBF_Instance init
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_SEEED_MONOCHROME_GROVE_16x2_LCD;
	// PnP is interrupt driven, no polling is needed. And this is an output-only device.
	this->pollIntervalMs = EBF_NO_POLLING;

	// Attach interrupt lines for that device
	rc = pAssignedHub->AssignInterruptLines(pPnPI2C->GetPortNumber(), endpointIndex, deviceInfo);
	if (rc != EBF_OK) {
		return rc;
	}

	return rc;
}
