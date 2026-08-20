#include "PnP_Module_8Outputs.h"

PnP_Module_8Outputs::PnP_Module_8Outputs() : EBF_Module_8Outputs(NULL)
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_8OUTPUTS;

	memset(pInterfaces, 0, sizeof(PnP_OutputInterface*) * numberOfOutputs);
}

uint8_t PnP_Module_8Outputs::Init()
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

	chip.pI2C = pPnPI2C;

	// Initialize the device
	rc = EBF_Module_8Outputs::Init(deviceInfo.endpointData[endpointIndex].i2cAddress);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	// Fix type and ID after the EBF_Instance init
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_8OUTPUTS;

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

uint8_t PnP_Module_8Outputs::Process()
{
	uint8_t rc;

	for (uint8_t i=0; i<numberOfOutputs; i++) {
		if (pInterfaces[i] != NULL) {
			rc = pInterfaces[i]->Process();
			if (rc != EBF_OK) {
				return rc;
			}
		}
	}

	return EBF_OK;
}

uint8_t PnP_Module_8Outputs::AssignInterface(uint8_t index, PnP_OutputInterface* pIfInstance)
{
	if (index >= numberOfOutputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	if (pInterfaces[index] != NULL) {
		EBF_REPORT_ERROR(EBF_INVALID_STATE);
		return EBF_INVALID_STATE;
	}

	pInterfaces[index] = pIfInstance;

	return pIfInstance->AssignInterfaceProvider(this, index);
}

uint8_t PnP_Module_8Outputs::SetValue_OIP(uint8_t index, float value)
{
	uint8_t rc;

	if (index >= numberOfOutputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	if (value < 0.0) value = 0.0;
	if (value > 100.0) value = 100.0;

	// Set PWM will handle 0% and 100% special cases
	rc = SetPWM(index, value);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

float PnP_Module_8Outputs::GetValue_OIP(uint8_t index)
{
	if (index >= numberOfOutputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return 0.0;
	}

	return GetValue(index);
}

// This is an override of the default function
void PnP_Module_8Outputs::SetPollingInterval(uint32_t ms)
{
	// Since we have multiple interfaces that might need the polling at the same time
	// we can't just change the value to NO_POLLING.
	// Need to check if there is an interface instance that might still need a lower value
	if (ms == EBF_NO_POLLING) {
		for (uint8_t i=0; i<numberOfOutputs; i++) {
			PnP_OutputInterface* pOutput = pInterfaces[i];

			// The instance still need processing
			if (pOutput->IsProcessingNeeded()) {
				return;
			}
		}
	}

	// Update the polling interval if requested time is lower than current or NO_POLLING is needed
	if (EBF_HalInstance::GetPollingInterval() > ms || ms == EBF_NO_POLLING) {
		EBF_HalInstance::SetPollingInterval(ms);
	}
}
