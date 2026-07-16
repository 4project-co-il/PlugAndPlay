#include "PnP_Module_4Inputs.h"

PnP_Module_4Inputs::PnP_Module_4Inputs() : EBF_Module_4Input(NULL)
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_4INPUTS;
}

uint8_t PnP_Module_4Inputs::Init()
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

	chip.pI2C = pPnPI2C;

	// Initialize the device
	rc = EBF_Module_4Input::Init(deviceInfo.endpointData[endpointIndex].i2cAddress);
	if (rc != EBF_OK) {
		return rc;
	}

	// Fix type and ID after the EBF_Instance init
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_4INPUTS;

	// PnP is interrupt driven, no polling is needed
	this->SetPollingInterval(EBF_NO_POLLING);

	// Attach interrupt lines for that device
	rc = pAssignedHub->AssignInterruptLines(pPnPI2C->GetPortNumber(), endpointIndex, deviceInfo);
	if (rc != EBF_OK) {
		return rc;
	}

	return rc;
}

void PnP_Module_4Inputs::ExecuteCallback()
{
	PnP_InputInterface::pCurrentProvider = this;

	if (isInterfaceAssigned & 1<<currentEventIndex) {
		// The onChangeCallback should be treated as a pointer to an interface instance
		PnP_InputInterface* pInput = GetAsInputInterface(currentEventIndex);

		pInput->ExecuteCallback();
	} else {
		// Interface is not assigned to that input, procceed with regular processing
		EBF_Module_4Input::ExecuteCallback();
	}

}

// Called by the EBF from normal run to take care of the events
uint8_t PnP_Module_4Inputs::Process()
{
	uint8_t rc;

	rc = EBF_Module_4Input::Process();
	if (rc != EBF_OK) {
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

uint8_t PnP_Module_4Inputs::AssignInterface(uint8_t index, PnP_InputInterface* pIfInstance)
{
	if (index >= numberOfInputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	onChangeCallback[index] = (EBF_CallbackType)pIfInstance;
	isInterfaceAssigned |= 1<<index;

	pIfInstance->SetInitialValue(EBF_Module_4Input::GetLastValue(index));

	return pIfInstance->AssignInterfaceProvider(this, index);
}

// Returns pointer to current interface instance, if it was assigned
PnP_InputInterface* PnP_Module_4Inputs::GetCurrentInterface()
{
	if (isInterfaceAssigned & 1<<currentEventIndex) {
		return GetAsInputInterface(currentEventIndex);
	}

SerialUSB.print("IDX=");
SerialUSB.println(currentEventIndex);
	return NULL;
}
