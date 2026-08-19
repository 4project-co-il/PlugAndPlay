#include "PnP_Module_8Inputs.h"

PnP_Module_8Inputs::PnP_Module_8Inputs() : EBF_Module_8Inputs(NULL)
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_8INPUTS;

	this->isInterfaceAssigned = 0;
}

uint8_t PnP_Module_8Inputs::Init()
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
	rc = EBF_Module_8Inputs::Init(deviceInfo.endpointData[endpointIndex].i2cAddress);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	// Fix type and ID after the EBF_Instance init
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_8INPUTS;

	// PnP is interrupt driven, no polling is needed
	this->SetPollingInterval(EBF_NO_POLLING);

	// Attach interrupt lines for that device
	rc = pAssignedHub->AssignInterruptLines(pPnPI2C->GetPortNumber(), endpointIndex, deviceInfo);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

void PnP_Module_8Inputs::ExecuteCallback()
{
	PnP_InputInterface::pCurrentProvider = this;

	if (isInterfaceAssigned & 1<<currentEventIndex) {
		// The onChangeCallback should be treated as a pointer to an interface instance
		PnP_InputInterface* pInput = GetAsInputInterface(currentEventIndex);

		pInput->ExecuteCallback();
	} else {
		// Interface is not assigned to that input, procceed with regular processing
		EBF_Module_8Inputs::ExecuteCallback();
	}

}

// Called by the EBF from normal run to take care of the events
uint8_t PnP_Module_8Inputs::Process()
{
	uint8_t rc;

	rc = EBF_Module_8Inputs::Process();
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	// Process call is relevant only to the assigned input interfaces (long-press for example)
	if (isInterfaceAssigned != 0) {
		for (currentEventIndex=0; currentEventIndex<numberOfInputs; currentEventIndex++) {
			// Call the processing function of input interface instance
			if (isInterfaceAssigned & 1<<currentEventIndex) {
				// Set current interface provider and event index before the callbacks are called
				PnP_InputInterface::pCurrentProvider = this;
				// currentEventIndex is advanced in the loop

				PnP_InputInterface* pInput = GetAsInputInterface(currentEventIndex);

				pInput->Process();
			}
		}
	}

	return EBF_OK;
}

uint8_t PnP_Module_8Inputs::AssignInterface(uint8_t index, PnP_InputInterface* pIfInstance)
{
	if (index >= numberOfInputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	if (isInterfaceAssigned & 1<<index) {
		EBF_REPORT_ERROR(EBF_INVALID_STATE);
		return EBF_INVALID_STATE;
	}

	onChangeCallback[index] = (EBF_CallbackType)pIfInstance;
	isInterfaceAssigned |= 1<<index;

	pIfInstance->SetInitialValue(GetLastValue(index));

	return pIfInstance->AssignInterfaceProvider(this, index);
}

// Returns pointer to current interface instance, if it was assigned
PnP_InputInterface* PnP_Module_8Inputs::GetCurrentInterface()
{
	if (isInterfaceAssigned & 1<<currentEventIndex) {
		return GetAsInputInterface(currentEventIndex);
	}

	return NULL;
}

// This is an override of the default function
void PnP_Module_8Inputs::SetPollingInterval(uint32_t ms)
{
	// Since we have multiple interfaces that might need the polling at the same time
	// we can't just change the value to NO_POLLING.
	// Need to check if there is an interface instance that might still need a lower value
	if (ms == EBF_NO_POLLING && isInterfaceAssigned != 0) {
		for (uint8_t i=0; i<numberOfInputs; i++) {
			if (isInterfaceAssigned & 1<<i) {
				PnP_InputInterface* pInput = GetAsInputInterface(i);

				// The input instance still need processing
				if (pInput->IsProcessingNeeded()) {
					return;
				}
			}
		}
	}

	// Update the polling interval if requested time is lower than current or NO_POLLING is needed
	if (EBF_HalInstance::GetPollingInterval() > ms || ms == EBF_NO_POLLING) {
		EBF_HalInstance::SetPollingInterval(ms);
	}
}
