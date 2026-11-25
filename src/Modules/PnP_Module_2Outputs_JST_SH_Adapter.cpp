#include "PnP_Module_2Outputs_JST_SH_Adapter.h"

PnP_Module_2Outputs_JST_SH_Adapter::PnP_Module_2Outputs_JST_SH_Adapter()
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_2OUTPUTS_JST_SH_ADAPTER;

	memset(pInterfaces, 0, sizeof(PnP_OutputInterface*) * numberOfOutputs);
}

uint8_t PnP_Module_2Outputs_JST_SH_Adapter::Init()
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
	this->id = PnP_DeviceId::PNP_ID_2OUTPUTS_JST_SH_ADAPTER;

	// PnP is interrupt driven, no polling is needed
	this->SetPollingInterval(EBF_NO_POLLING);

	// Attach interrupt lines for that device
	// Current device don't produce interrupts, but all the initializations are done in AssignInterruptLines
	rc = pAssignedHub->AssignInterruptLines(pPnPI2C->GetPortNumber(), endpointIndex, deviceInfo);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	// Start with Off value
	rc = Off(0);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	rc = Off(1);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_Module_2Outputs_JST_SH_Adapter::Process()
{
	uint8_t rc;

	for (uint8_t i=0; i<numberOfOutputs; i++) {
		if (pInterfaces[i] != NULL) {
			rc = pInterfaces[i]->Process();
			if (rc != EBF_OK) {
				EBF_REPORT_AND_RETURN(rc);
			}
		}
	}

	EBF_REPORT_AND_RETURN(EBF_OK);
}

// Set both interrupt lines values
uint8_t PnP_Module_2Outputs_JST_SH_Adapter::SetValues(uint8_t values)
{
	uint8_t rc;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();

	rc = pHub->SetIntLinesValue(pPnPI2C->GetPortNumber(), values & 0x03);

	EBF_REPORT_AND_RETURN(rc);
}

// Turns the output ON.
uint8_t PnP_Module_2Outputs_JST_SH_Adapter::On(uint8_t index)
{
	uint8_t rc;

	if (index >= numberOfOutputs) {
		EBF_REPORT_AND_RETURN(EBF_INDEX_OUT_OF_BOUNDS);
	}

	rc = SetIntLine(pPnPI2C, index, 1);

	EBF_REPORT_AND_RETURN(rc);
}

// Turns the otuput OFF.
uint8_t PnP_Module_2Outputs_JST_SH_Adapter::Off(uint8_t index)
{
	uint8_t rc;

	if (index >= numberOfOutputs) {
		EBF_REPORT_AND_RETURN(EBF_INDEX_OUT_OF_BOUNDS);
	}

	rc = SetIntLine(pPnPI2C, index, 0);

	EBF_REPORT_AND_RETURN(rc);
}

// Set value of specified index
uint8_t PnP_Module_2Outputs_JST_SH_Adapter::SetValue(uint8_t index, uint8_t value)
{
	uint8_t rc;

	if (index >= numberOfOutputs) {
		EBF_REPORT_AND_RETURN(EBF_INDEX_OUT_OF_BOUNDS);
	}

	if (value == 0) {
		rc = SetIntLine(pPnPI2C, index, 0);
	} else {
		rc = SetIntLine(pPnPI2C, index, 1);
	}

	EBF_REPORT_AND_RETURN(rc);
}

// Returns 1 if output is ON, 0 if OFF
uint8_t PnP_Module_2Outputs_JST_SH_Adapter::GetValue(uint8_t index)
{
	uint8_t rc;
	uint8_t value;

	if (index >= numberOfOutputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return 0;
	}

	rc = GetIntLine(pPnPI2C, index, value);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return 0;
	}

	if (value == 0) {
		return 0;
	} else {
		return 1;
	}
}

uint8_t PnP_Module_2Outputs_JST_SH_Adapter::AssignInterface(uint8_t index, PnP_OutputInterface* pIfInstance)
{
	uint8_t rc;

	if (index >= numberOfOutputs) {
		EBF_REPORT_AND_RETURN(EBF_INDEX_OUT_OF_BOUNDS);
	}

	// Only relay interface is accepted here
	if (pIfInstance->GetType() != PnP_OutputInterface::RELAY) {
		EBF_REPORT_AND_RETURN(EBF_INVALID_STATE);
	}

	if (pInterfaces[index] != NULL) {
		EBF_REPORT_AND_RETURN(EBF_INVALID_STATE);
	}

	pInterfaces[index] = pIfInstance;

	rc = pIfInstance->AssignInterfaceProvider(this, index);

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_Module_2Outputs_JST_SH_Adapter::SetValue_OIP(uint8_t index, float value)
{
	uint8_t rc;

	if (index >= numberOfOutputs) {
		EBF_REPORT_AND_RETURN(EBF_INDEX_OUT_OF_BOUNDS);
	}

	if (value == 0.0) {
		rc = SetValue(index, (uint8_t)0);
	} else {
		rc = SetValue(index, (uint8_t)1);
	}

	EBF_REPORT_AND_RETURN(rc);
}

float PnP_Module_2Outputs_JST_SH_Adapter::GetValue_OIP(uint8_t index)
{
	if (index >= numberOfOutputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return 0.0;
	}

	if (GetValue(index) == 0) {
		return 0.0;
	} else {
		return 100.0;
	}
}

// This is an override of the default function
void PnP_Module_2Outputs_JST_SH_Adapter::SetPollingInterval(uint32_t ms)
{
	uint32_t interval = EBF_NO_POLLING;

	// Since we have multiple interfaces that might need different polling intrevals,
	// we'll look for the lowest value and ignore the passed value
	for (uint8_t i=0; i<numberOfOutputs; i++) {
		PnP_OutputInterface* pOutput = pInterfaces[i];

		// The instance still need processing
		if (pOutput != NULL && pOutput->GetPollingInterval() < interval) {
			interval = pOutput->GetPollingInterval();
		}
	}

	// Update the polling interval to the lowest found value
	EBF_HalInstance::SetPollingInterval(interval);
}
