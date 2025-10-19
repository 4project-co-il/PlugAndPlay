#include "PnP_Module_1Input.h"

PnP_Module_1Input::PnP_Module_1Input()
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_1INPUT;
}

extern void EBF_EmptyCallback();

uint8_t PnP_Module_1Input::Init()
{
	uint8_t rc;
	PnP_DeviceInfo deviceInfo;
	uint8_t endpointIndex;
	PnP_PlugAndPlayI2C *pPnPI2C;
	PnP_PlugAndPlayHub *pAssignedHub;


	PnP_PlugAndPlayManager *pPnpManager = PnP_PlugAndPlayManager::GetInstance();

	onChangeCallback = EBF_EmptyCallback;

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
	rc = EBF_HalInstance::Init(HAL_Type::PnP_DEVICE, PnP_DeviceId::PNP_ID_1INPUT);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	// PnP is interrupt driven, no polling is needed
	this->pollIntervalMs = EBF_NO_POLLING;

	// Attach interrupt lines for that device
	rc = pAssignedHub->AssignInterruptLines(pPnPI2C->GetPortNumber(), endpointIndex, deviceInfo);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

uint8_t PnP_Module_1Input::Process()
{
	uint8_t intBits;
	EBF_Logic *pLogic = EBF_Logic::GetInstance();

	// Process interrupt detected logic
	if (pLogic->IsPostInterruptProcessing()) {
		intBits = pLogic->GetLastMessageParam1();

		if(intBits & 0x01) {
			onChangeCallback();
		}
	}

	return EBF_OK;
}

// Returns 1 if input is HIGH
uint8_t PnP_Module_1Input::GetValue()
{
	uint8_t rc;
	uint8_t value;

	rc = GetIntLine(0, value);
	if (rc != EBF_OK) {
		return 0;
	}

	return value;
}

uint8_t PnP_Module_1Input::GetIntLine(uint8_t line, uint8_t &value)
{
	uint8_t rc = EBF_OK;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();

	rc = pHub->GetIntLine(pPnPI2C, pPnPI2C->GetPortNumber(), line, value);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

void PnP_Module_1Input::ProcessInterrupt()
{
	onChangeCallback();
}

// PostponeProcessing should be called to postpone the callback processing later in the normal loop
uint8_t PnP_Module_1Input::PostponeProcessing()
{
	uint8_t rc;
	EBF_Logic *pLogic = EBF_Logic::GetInstance();

	// Pass the control back to EBF, so it will call the Process() function from normal run

	// 0x01 = first bit set, indicating interrupt #1 fired
	rc = pLogic->PostponeInterrupt(this, 0x01);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}
