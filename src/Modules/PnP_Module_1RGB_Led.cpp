#include "PnP_Module_1RGB_Led.h"

PnP_Module_1RGB_Led::PnP_Module_1RGB_Led() : EBF_Module_1RGB_Led(NULL)
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_1_RGB_LED_LP5815;
}

uint8_t PnP_Module_1RGB_Led::Init()
{
	uint8_t rc = EBF_OK;
	PnP_DeviceInfo deviceInfo;
	uint8_t endpointIndex;
	PnP_PlugAndPlayI2C *pPnPI2C;
	PnP_PlugAndPlayHub *pAssignedHub;

	PnP_PlugAndPlayManager *pPnpManager = PnP_PlugAndPlayManager::GetInstance();

	// TODO: Retrieve device parameters and pass it's data to Init the module with current limits

	// Assign the current instance to physical PnP device and get all needed information
	rc = pPnpManager->AssignDevice(this, deviceInfo, endpointIndex, &pPnPI2C, &pAssignedHub);
	if(rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	chip.pI2C = pPnPI2C;

	// Initialize the device
	rc = EBF_Module_1RGB_Led::Init(deviceInfo.endpointData[endpointIndex].i2cAddress);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	// Fix type and ID after the EBF_Instance init
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_1_RGB_LED_LP5815;

	// PnP is interrupt driven, no polling is needed
	this->SetPollingInterval(EBF_NO_POLLING);

	// Attach interrupt lines for that device
	rc = pAssignedHub->AssignInterruptLines(pPnPI2C->GetPortNumber(), endpointIndex, deviceInfo);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	// Set the INT1 line to "1" logic to prevent driver's shutdown
	rc = SetIntLine(0, 1);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_Module_1RGB_Led::PowerDown()
{
	uint8_t rc;

	// Setnd power down command to the chip
	rc = chip.ShutdownCommand();
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	// And lower the INT1 line
	rc = SetIntLine(0, 0);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_Module_1RGB_Led::PowerUp()
{
	uint8_t rc;

	// Raise the INT1 line to wake up the chip
	rc = SetIntLine(0, 1);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	// TODO: read the device parameters to get LED current limits

	// Init the chip
	//chip.Init(chip.defaultI2CAddress, 0, ..)

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_Module_1RGB_Led::SetIntLine(uint8_t line, uint8_t value)
{
	uint8_t rc;
	PnP_PlugAndPlayHub *pHub = ((PnP_PlugAndPlayI2C*)chip.pI2C)->GetHub();

	// Line can be only 0 or 1 (the interrupt line number)
	if (line > 1) {
		EBF_REPORT_AND_RETURN(EBF_INDEX_OUT_OF_BOUNDS);
	}

	rc = pHub->SetIntLine(((PnP_PlugAndPlayI2C*)chip.pI2C)->GetPortNumber(), line, value & 0x03);

	EBF_REPORT_AND_RETURN(rc);
}