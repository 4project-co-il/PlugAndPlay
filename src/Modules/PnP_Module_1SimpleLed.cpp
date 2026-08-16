#include "PnP_Module_1SimpleLed.h"

PnP_Module_1SimpleLed::PnP_Module_1SimpleLed()
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_1_SIMPLE_LED;

	pInterface = NULL;
	outputValue = 0;
}

uint8_t PnP_Module_1SimpleLed::Init()
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
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	// Save the I2C instance, although this device doesn't communicate via I2C, but via the HUBs
	// The PlugAndPlayI2C class have pointer to the HUB and port number, which are needed for interrupt lines manipulation
	this->pPnPI2C = pPnPI2C;

	// Initialize the instance
	rc = EBF_HalInstance::Init(this->type, this->id);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	// PnP is interrupt driven, no polling is needed
	this->SetPollingInterval(EBF_NO_POLLING);

	// Attach interrupt lines for that device
	// Current device don't produce interrupts, but all the initializations are done in AssignInterruptLines
	rc = pAssignedHub->AssignInterruptLines(pPnPI2C->GetPortNumber(), endpointIndex, deviceInfo);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

uint8_t PnP_Module_1SimpleLed::Process()
{
	if (pInterface != NULL) {
		return pInterface->Process();
	}

	return EBF_OK;
}

// Turns the LED ON.
uint8_t PnP_Module_1SimpleLed::On()
{
	outputValue = 1;

	return SetIntLine(0, 1);
}

// Turns the LED OFF.
uint8_t PnP_Module_1SimpleLed::Off()
{
	outputValue = 0;

	return SetIntLine(0, 0);
}

// Sets current LED value
uint8_t PnP_Module_1SimpleLed::SetValue(uint8_t value)
{
	outputValue = value;

	return SetIntLine(0, value);
}

uint8_t PnP_Module_1SimpleLed::SetIntLine(uint8_t line, uint8_t value)
{
	uint8_t rc = EBF_OK;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();

	// Line can be only 0 or 1 (the interrupt line number)
	if (line > 1) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	rc = pHub->SetIntLine(pPnPI2C->GetPortNumber(), line, value & 0x03);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return rc;
}

uint8_t PnP_Module_1SimpleLed::AssignInterface(PnP_OutputInterface* pIfInstance)
{
	// Only simple LED interface is accepted here
	if (pIfInstance->GetType() != PnP_OutputInterface::SIMPLE_LED) {
		return EBF_INVALID_STATE;
	}

	pInterface = pIfInstance;

	return pIfInstance->AssignInterfaceProvider(this, 0);
}

uint8_t PnP_Module_1SimpleLed::SetValue_OIP(uint8_t index, float value)
{
	if (index != 0) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	if (value == 0.0) {
		return SetValue((uint8_t)0);
	} else {
		return SetValue((uint8_t)1);
	}
}

float PnP_Module_1SimpleLed::GetValue_OIP(uint8_t index)
{
	if (index != 0) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return 0.0;
	}

	return (float)outputValue;
}
