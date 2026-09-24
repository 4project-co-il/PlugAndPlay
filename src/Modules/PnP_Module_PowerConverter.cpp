#include "PnP_Module_PowerConverter.h"

PnP_Module_PowerConverter::PnP_Module_PowerConverter()
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_POWER_CONVERTER_CONTROL;
}

uint8_t PnP_Module_PowerConverter::Init(uint8_t enable)
{
	uint8_t rc;
	PnP_DeviceInfo deviceInfo;
	uint8_t endpointIndex;
	PnP_PlugAndPlayI2C *pPnPI2C;
	PnP_PlugAndPlayHub *pAssignedHub;

	PnP_PlugAndPlayManager *pPnpManager = PnP_PlugAndPlayManager::GetInstance();

	// Assign the current instance to physical PnP device and get all needed information
	rc = pPnpManager->AssignDevice(this, deviceInfo, endpointIndex, &pPnPI2C, &pAssignedHub);
	if(rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	// Save the I2C instance, although this device doesn't communicate via I2C, but via the HUBs
	// The PlugAndPlayI2C class have pointer to the HUB and port number, which are needed for interrupt lines manipulation
	this->pPnPI2C = pPnPI2C;

	// Initialize the instance
	rc = EBF_HalInstance::Init(this->type, this->id);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	// Fix type and ID after the EBF_Instance init
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_POWER_CONVERTER_CONTROL;

	// PnP is interrupt driven, no polling is needed
	this->SetPollingInterval(EBF_NO_POLLING);

	// Attach interrupt lines for that device
	// Current device don't produce interrupts, but all the initializations are done in AssignInterruptLines
	rc = pAssignedHub->AssignInterruptLines(pPnPI2C->GetPortNumber(), endpointIndex, deviceInfo);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	if (enable) {
		rc = Enable();
	} else {
		rc = Disable();
	}

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_Module_PowerConverter::Enable()
{
	EBF_REPORT_AND_RETURN(SetIntLine(0, 1));
}

uint8_t PnP_Module_PowerConverter::Disable()
{
	EBF_REPORT_AND_RETURN(SetIntLine(0, 0));
}

uint8_t PnP_Module_PowerConverter::IsPowerGood()
{
	uint8_t rc;
	uint8_t value;

	rc = GetIntLine(1, value);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return 0;
	}

	return value;
}

uint8_t PnP_Module_PowerConverter::IsEnabled()
{
	uint8_t rc;
	uint8_t value;

	rc = GetIntLine(0, value);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return 0;
	}

	return value;
}

uint8_t PnP_Module_PowerConverter::SetIntLine(uint8_t line, uint8_t value)
{
	uint8_t rc = EBF_OK;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();

	// Line can be only 0 or 1 (the interrupt line number)
	if (line > 1) {
		EBF_REPORT_AND_RETURN(EBF_INDEX_OUT_OF_BOUNDS);
	}

	rc = pHub->SetIntLine(pPnPI2C->GetPortNumber(), line, value & 0x03);

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_Module_PowerConverter::GetIntLine(uint8_t line, uint8_t &value)
{
	uint8_t rc;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();

	rc = pHub->GetIntLine(pPnPI2C->GetPortNumber(), line, value);

	EBF_REPORT_AND_RETURN(rc);
}
