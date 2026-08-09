#include "PnP_Module_2Inputs.h"

extern void EBF_EmptyCallback();

PnP_Module_2Inputs::PnP_Module_2Inputs()
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_2INPUTS;

	isInterfaceAssigned = 0;
	lastValues = 0;
	currentEventIndex = 0;

	for (uint8_t i=0; i<numberOfInputs; i++) {
		onChangeCallback[i] = EBF_EmptyCallback;
	}
}

uint8_t PnP_Module_2Inputs::Init()
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
	rc = pAssignedHub->AssignInterruptLines(pPnPI2C->GetPortNumber(), endpointIndex, deviceInfo);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	// Read initial input status
	lastValues = GetValues();

	return EBF_OK;
}

// Called by the EBF from normal run to take care of the events
uint8_t PnP_Module_2Inputs::Process()
{
	EBF_Logic *pLogic = EBF_Logic::GetInstance();
	PostponedInterruptData data = {0};

	// Process interrupt detected logic
	if (pLogic->IsPostInterruptProcessing()) {
		data.uint32 = pLogic->GetLastMessageParam1();

		// Set current interface provider and event index before the callbacks are called
		PnP_InputInterface::pCurrentProvider = this;
		currentEventIndex = data.fields.index;
		lastValues = data.fields.event;

		if (isInterfaceAssigned & 1<<data.fields.index) {
			// The onChangeCallback should be treated as a pointer to an interface instance
			PnP_InputInterface* pInput = GetAsInputInterface(data.fields.index);

			pInput->ExecuteCallback();
		} else {
			// Callback from the normal run mode
			onChangeCallback[data.fields.index]();
		}
	}

	// Processing is relevant only to the assigned input interfaces (long-press for example)
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

// Returns 1 if input is HIGH for a specific interrupt line
uint8_t PnP_Module_2Inputs::GetValue(uint8_t index)
{
	uint8_t rc;
	uint8_t value;

	rc = GetIntLine(index, value);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return 0;
	}

	return value;
}

// Returns bits 0 and 1 as HIGH or LOW for both interrupt lines
uint8_t PnP_Module_2Inputs::GetValues()
{
	uint8_t rc;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();
	uint8_t value;

	rc = pHub->GetIntLinesValue(pPnPI2C->GetPortNumber(), value);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return 0;
	}

	return value;

}

// Returns the value of the input line as it was registered during last interrupt
uint8_t PnP_Module_2Inputs::GetLastValue(uint8_t index)
{
	if (lastValues & 1<<index) {
		return 1;
	} else {
		return 0;
	}
}

// Returns the value of the input line as it was registered during last interrupt
uint8_t PnP_Module_2Inputs::GetLastValues()
{
	return lastValues;
}

// Returns pointer to current interface instance, if it was assigned
PnP_InputInterface* PnP_Module_2Inputs::GetCurrentInterface()
{
	if (isInterfaceAssigned & 1<<currentEventIndex) {
		return GetAsInputInterface(currentEventIndex);
	}

	return NULL;
}

uint8_t PnP_Module_2Inputs::GetIntLine(uint8_t line, uint8_t &value)
{
	uint8_t rc;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();

	rc = pHub->GetIntLine(pPnPI2C->GetPortNumber(), line, value);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

// Called directly from the ISR
void PnP_Module_2Inputs::ProcessInterrupt()
{
	EBF_Logic *pLogic = EBF_Logic::GetInstance();
	PnP_PlugAndPlayHub::InterruptHint hint;
	uint8_t inputs = 0;

	// Hint will tell us what interrupt arrived
	hint.uint32 = pLogic->GetInterruptHint();

	inputs = GetValues();

#ifdef EBF_DIRECT_CALL_FROM_ISR
	// Set current interface provider and event index before the callbacks are called
	PnP_InputInterface::pCurrentProvider = this;
	currentEventIndex = hint.fields.interruptNumber;
	lastValues = inputs;

	if (isInterfaceAssigned & 1<<hint.fields.interruptNumber) {
		// The onChangeCallback should be treated as a pointer to an interface instance
		PnP_InputInterface* pInput = GetAsInputInterface(hint.fields.interruptNumber);

		pInput->ExecuteCallback();
	} else {
		onChangeCallback[hint.fields.interruptNumber]();
	}
#else
	// Postpone the processing so the event will be handled from the normal run
	PostponeProcessing(hint.fields.interruptNumber, inputs);
#endif
}

// PostponeProcessing should be called to execute the callback processing later in the normal loop
uint8_t PnP_Module_2Inputs::PostponeProcessing(uint8_t eventIndex, uint8_t inputValues)
{
	uint8_t rc;
	EBF_Logic *pLogic = EBF_Logic::GetInstance();
	PostponedInterruptData data = {0};

	data.fields.index = eventIndex;
	data.fields.event = inputValues;

	// Pass the control back to EBF, so it will call the Process() function from normal run
	rc = pLogic->PostponeInterrupt(this, data.uint32);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

uint8_t PnP_Module_2Inputs::SetOnChange(uint8_t index, EBF_CallbackType onChangeCallback)
{
	if (index >= numberOfInputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	this->onChangeCallback[index] = onChangeCallback;

	return EBF_OK;
}

uint8_t PnP_Module_2Inputs::AssignInterface(uint8_t index, PnP_InputInterface* pIfInstance)
{
	if (index >= numberOfInputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	onChangeCallback[index] = (EBF_CallbackType)pIfInstance;
	isInterfaceAssigned |= 1<<index;

	pIfInstance->SetInitialValue(GetLastValue(index));

	return pIfInstance->AssignInterfaceProvider(this, index);
}

// This is an override of the default function
void PnP_Module_2Inputs::SetPollingInterval(uint32_t ms)
{
	// Since we have multiple inputs that might need the polling at the same time
	// we can't just change the value. Need to check if there is an interface instance that might
	// still need the low value
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

	EBF_HalInstance::SetPollingInterval(ms);
}

// Returns input index that caused the callback function call
// You can have the same callback function for all onChange events
// where you can call the GetEventIndex to know which input actually changed
uint8_t PnP_Module_2Inputs::GetEventIndex()
{
	EBF_Logic *pLogic = EBF_Logic::GetInstance();
	PnP_PlugAndPlayHub::InterruptHint hint;

	if (InInterrupt()) {
		// Hint will tell us what interrupt arrived
		hint.uint32 = pLogic->GetInterruptHint();

		return hint.fields.interruptNumber;
	} else {
		return currentEventIndex;
	}
}