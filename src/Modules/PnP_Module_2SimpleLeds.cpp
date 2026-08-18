#include "PnP_Module_2SimpleLeds.h"

PnP_Module_2SimpleLeds::PnP_Module_2SimpleLeds()
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_2_SIMPLE_LEDS;

	memset(pInterfaces, 0, sizeof(PnP_OutputInterface*) * numberOfOutputs);

	outputValue = 0;
}

uint8_t PnP_Module_2SimpleLeds::Init()
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

uint8_t PnP_Module_2SimpleLeds::Process()
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

// Set both interrupt lines values
uint8_t PnP_Module_2SimpleLeds::SetValue(uint8_t value)
{
	uint8_t rc;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();

	// Clear 2 last bits
	outputValue &= 0xFC;
	// Set the last 2 bits that represent the 2 LEDs
	outputValue |= (value & 0x3);

	rc = pHub->SetIntLinesValue(pPnPI2C->GetPortNumber(), value);
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

// Turns the LED ON.
uint8_t PnP_Module_2SimpleLeds::On(uint8_t index)
{
	outputValue &= ~(1<<index);
	outputValue |= 1<<index;

	return SetIntLine(index, 1);
}

// Turns the LED OFF.
uint8_t PnP_Module_2SimpleLeds::Off(uint8_t index)
{
	outputValue &= ~(1<<index);
	outputValue |= 1<<index;

	return SetIntLine(index, 0);
}

uint8_t PnP_Module_2SimpleLeds::SetIntLine(uint8_t line, uint8_t value)
{
	uint8_t rc;
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

	return EBF_OK;
}

uint8_t PnP_Module_2SimpleLeds::AssignInterface(uint8_t index, PnP_OutputInterface* pIfInstance)
{
	if (index >= numberOfOutputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	// Only simple LED interface is accepted here
	if (pIfInstance->GetType() != PnP_OutputInterface::SIMPLE_LED) {
		EBF_REPORT_ERROR(EBF_INVALID_STATE);
		return EBF_INVALID_STATE;
	}

	if (pInterfaces[index] != NULL) {
		EBF_REPORT_ERROR(EBF_INVALID_STATE);
		return EBF_INVALID_STATE;
	}

	pInterfaces[index] = pIfInstance;

	return pIfInstance->AssignInterfaceProvider(this, index);
}

uint8_t PnP_Module_2SimpleLeds::SetValue_OIP(uint8_t index, float value)
{
	if (index >= numberOfOutputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	if (value == 0.0) {
		return SetValue((uint8_t)0);
	} else {
		return SetValue((uint8_t)1);
	}
}

float PnP_Module_2SimpleLeds::GetValue_OIP(uint8_t index)
{
	if (index >= numberOfOutputs) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return 0.0;
	}

	if ((outputValue & 1<<index) == 0) {
		return 0.0;
	} else {
		return 1.0;
	}
}

// This is an override of the default function
void PnP_Module_2SimpleLeds::SetPollingInterval(uint32_t ms)
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
