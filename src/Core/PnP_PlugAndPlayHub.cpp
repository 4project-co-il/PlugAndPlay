#include "PnP_PlugAndPlayHub.h"

PnP_PlugAndPlayHub::PnP_PlugAndPlayHub(EBF_I2C* pI2C) : intControllerChip(pI2C), i2cSwitchChip(pI2C)
{
	this->type = HAL_Type::PnP_DEVICE;

	memset(portInterruptMode, 0, sizeof(PnP_InterruptMode) * maxPorts);

	// Reset the interrupt controller and I2C switch default I2C addresses
	// Those will be filled by the deviceInfo information read from the HUB
	intControllerChip.i2cAddress = 0;
	i2cSwitchChip.i2cAddress = 0;
	pPortInfo = NULL;
	pParentHub = NULL;

	lastInputs = 0xFFFF;
}

uint8_t PnP_PlugAndPlayHub::Init(PnP_PlugAndPlayHub *pParentHub, uint8_t parentPort, PnP_DeviceInfo &deviceInfo, uint8_t *pParams)
{
	uint8_t rc;

	this->numberOfPorts = deviceInfo.numberOfPorts;
	this->hubInt1Mode = (PnP_InterruptMode)deviceInfo.interrupt1Mode;
	this->hubInt2Mode = (PnP_InterruptMode)deviceInfo.interrupt2Mode;

	// This class handles only the HUB devices
	if (deviceInfo.deviceIDs[0] != PnP_DeviceId::PNP_ID_EMBEDDED_HUB &&
		deviceInfo.deviceIDs[0] != PnP_DeviceId::PNP_ID_EXTENDER_HUB) {
		EBF_REPORT_ERROR(EBF_INVALID_STATE);
		return EBF_INVALID_STATE;
	}

	// Convert EBF_I2C used for I2C communication to PnP_PlugAndPlayI2C in order to utilize automatic port switching
	PnP_PlugAndPlayI2C* pPnPI2C = new PnP_PlugAndPlayI2C(*i2cSwitchChip.pI2C, pParentHub, parentPort);
	i2cSwitchChip.pI2C = pPnPI2C;
	intControllerChip.pI2C = pPnPI2C;

	this->pParentHub = pParentHub;
	this->parentPortNumber = parentPort;

	if (pParentHub == NULL) {
		// Main HUBs will be level 1
		routingLevel = PnP_PlugAndPlayManager::PNP_EEPROM_MAIN_HUB;
	} else {
		// Generic HUBs will be on the next levels (2,3)
		routingLevel = pParentHub->routingLevel + 1;
	}

	// Should not initialize HAL instance, so the HUB will not be registered in EBF_Logic HALs instances array
	// Since HUBs are created dynamically, there is no entry for them in the array, as it's initialized based
	// on known number of instances that are statically declared in main code.
	// The Process() function for every HAL instance (PnP module) will be called directly from the EBF_Logic
	// for every registered HAL instance.
//	rc = EBF_HalInstance::Init(HAL_Type::I2C_INTERFACE, parentPort);
//	if (rc != EBF_OK) {
//		EBF_REPORT_ERROR(rc);
//		return rc;
//	}

	// Fix type and ID after the EBF_Instance init
	this->type = HAL_Type::PnP_DEVICE;
	this->id = deviceInfo.deviceIDs[0];
	this->SetPollingInterval(EBF_NO_POLLING);	// No polling is needed for HUBs

	// Allocate pointers to HAL instances. Will be used to pass the interrrupts to connected instances
	// Allocate port info structure. HAL pointer will be used to pass the interrrupts to connected instances
	pPortInfo = (PortInfo*)malloc(sizeof(PortInfo) * numberOfPorts);
	if(pPortInfo == NULL) {
		EBF_REPORT_ERROR(EBF_NOT_ENOUGH_MEMORY);
		return EBF_NOT_ENOUGH_MEMORY;
	}

	memset(pPortInfo, 0, sizeof(PortInfo) * numberOfPorts);

	// We do not initialize port info data since we don't know what is connected to those ports yet

	// Endpoints specify switch chip and interrupt controller (if exist)
	// Addresses should be shifted by the routing level to prevent collision between the hubs
	for (uint8_t i=0; i<deviceInfo.numberOfEndpoints; i++) {
		if (deviceInfo.endpointData[i].endpointId == 0) {
			// 0 is for HUB's I2C switch
			i2cSwitchChip.i2cAddress = deviceInfo.endpointData[i].i2cAddress + this->routingLevel;

			// Reset the switching that might be left from previous run
			rc = i2cSwitchChip.Reset();
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}
		}

		if (deviceInfo.endpointData[i].endpointId == 1) {
			// 1 is for the HUB interrupt controller

			// Initialize the address of interrupt controller chip
			intControllerChip.i2cAddress = deviceInfo.endpointData[i].i2cAddress + this->routingLevel;

			// Configure the chip to all inputs, latch enable and all interrupts masked
			rc = intControllerChip.SetConfiguration(0xFFFF);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}

			rc = intControllerChip.SetLatching(0xFFFF);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}

			// Disable all 16bits. Specific line will be enabled based on connected devices
			rc = intControllerChip.SetInterruptMask(0xFFFF);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}

			// Get initial input lines status to reset the interrupts
			rc = intControllerChip.GetInput(lastInputs);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}
		}
	}

	memset(interruptMapping, 0, sizeof(interruptMapping));
	if (intControllerChip.i2cAddress == 0) {
		// For HUB that is directly connected to MCU's interrupt lines:

		// Copy interrupt mapping from the params, if specified
		// It will be used later by the embedded HUBs to connect needed interrrupt lines
		if (deviceInfo.paramsLength > 0) {
			memcpy(interruptMapping, pParams, deviceInfo.paramsLength);
		}
	} else {
		// For HUBs with interrupt controller, the interruptMapping will store endpoint numbers for every interrupt
	}

	// For embedded HUBs with interrupt controllers, assign the interrupt lines on initialization
	// Embedded HUBs without interrupt controller will assign the lines for every attached device
	if (this->GetId() == PnP_DeviceId::PNP_ID_EMBEDDED_HUB && intControllerChip.i2cAddress != 0) {
			rc = AssignInterruptLines(parentPort, 0, deviceInfo);
			if (rc != EBF_OK) {
				return rc;
			}
		}

	return EBF_OK;
}

uint8_t PnP_PlugAndPlayHub::AssignEmbeddedHubLine(uint8_t pinNumber, PnP_InterruptMode intMode, InterruptHint intHint)
{
	switch (intMode) {
		case PNP_NOT_CONECTED:
			// The line is not connected, nothing to do
			break;

		case PNP_DIGITAL_OUTPUT:
			// Initialize the line as digital output
			pinMode(pinNumber, OUTPUT);
			break;

		case PNP_DIGITAL_INPUT:
			// Initialize the line as digital input, but without an interrupt
			pinMode(pinNumber, INPUT);
			break;

		default:
			// For the rest of the cases attach the line as an interrupt

			// interrupt hint will include port number shifted one bit left and the LSB specifying
			// if it's the first interrupt for the device or the second
			if (pinNumber != (uint8_t)(-1)) {
				pinMode(pinNumber, INPUT);

				uint8_t rc;
				EBF_Logic *pLogic = EBF_Logic::GetInstance();

				rc = pLogic->AttachInterrupt(pinNumber, this, GetArduinoInterruptMode(intMode), intHint.uint32);

				if (rc != EBF_OK) {
					EBF_REPORT_ERROR(rc);
					return rc;
				}
			}
	}

	return EBF_OK;
}

uint8_t PnP_PlugAndPlayHub::AssignInterruptControllerLine(uint8_t intLine, PnP_InterruptMode intMode)
{
	uint8_t rc;

	switch (intMode)
	{
	case PnP_InterruptMode::PNP_NOT_CONECTED:
		// Nothing to do
		break;

	case PnP_InterruptMode::PNP_DIGITAL_OUTPUT:
		uint16_t config;

		// Change the line to output mode
		rc = intControllerChip.GetConfiguration(config);
		if (rc != EBF_OK) {
			return rc;
		}

		config &= ~(1<<intLine);

		rc = intControllerChip.SetConfiguration(config);
		if (rc != EBF_OK) {
			return rc;
		}

		break;

	case PnP_InterruptMode::PNP_DIGITAL_INPUT:
		// Nothing to do, all the lines are set to input by default during initialization
		break;

	case PnP_InterruptMode::PNP_INTERRUPT_ON_CHANGE:
	case PnP_InterruptMode::PNP_INTERRUPT_LOW:
	case PnP_InterruptMode::PNP_INTERRUPT_HIGH:
	case PnP_InterruptMode::PNP_INTERRUPT_RISING:
	case PnP_InterruptMode::PNP_INTERRUPT_FALLING:
		// Unmask the interrupt line
		uint16_t intMask;

		rc = intControllerChip.GetInterruptMask(intMask);
		if (rc != EBF_OK) {
			return rc;
		}

		intMask &= ~(1<<intLine);

		rc = intControllerChip.SetInterruptMask(intMask);
		if (rc != EBF_OK) {
			return rc;
		}

		break;

	default:
		// Should not happen
		return (uint8_t)(-1);
		break;
	}

	return EBF_OK;
}

uint8_t PnP_PlugAndPlayHub::AssignInterruptLines(uint8_t portNumber, uint8_t endpointNumber, PnP_DeviceInfo &deviceInfo)
{
	uint8_t rc;
	InterruptHint hint;
	PnP_InterruptMode int1Mode = PnP_InterruptMode::PNP_NOT_CONECTED;
	PnP_InterruptMode int2Mode = PnP_InterruptMode::PNP_NOT_CONECTED;

	if (deviceInfo.interrupt1Endpoint == endpointNumber) {
		int1Mode = (PnP_InterruptMode)deviceInfo.interrupt1Mode;
	}

	if (deviceInfo.interrupt2Endpoint == endpointNumber) {
		int2Mode = (PnP_InterruptMode)deviceInfo.interrupt2Mode;
	}

	// Save the interrupt modes for future usage
	portInterruptMode[portNumber*2 + 0] = int1Mode;
	portInterruptMode[portNumber*2 + 1] = int2Mode;

	// For embedded HUBs without interrupt controller, attach specified interrupts to the EBF logic
	// Additional parameters will specify ports to interrupt lines mapping for embedded HUBs
	if (this->GetId() == PnP_DeviceId::PNP_ID_EMBEDDED_HUB) {
		if (intControllerChip.i2cAddress == 0) {
			// Use the hint to check if the interrupt was already attached
			// Might happen when an extender HUB is used, so all the ports of the extender are assigned to the
			// same interrupt of the embedded HUB
			EBF_Logic *pLogic = EBF_Logic::GetInstance();

			hint.uint32 = pLogic->GetInterruptHint(interruptMapping[portNumber*2 + 0]);
			if (hint.fields.attached) {
				// We already attached that interrupt, no need to do it again
				return EBF_OK;
			}

			hint.uint32 = pLogic->GetInterruptHint(interruptMapping[portNumber*2 + 1]);
			if (hint.fields.attached) {
				// We already attached that interrupt, no need to do it again
				return EBF_OK;
			}

			// interrupt hint will include port number shifted one bit left and the LSB specifying
			// if it's the first interrupt for the device or the second
			// it helps passing the interrupt call to the correct HAL instance
			hint.uint32 = 0;
			hint.fields.interruptNumber = 0;
			hint.fields.portNumber = portNumber;
			hint.fields.endpointNumber = endpointNumber;
			hint.fields.attached = 1;

			rc = this->AssignEmbeddedHubLine(interruptMapping[portNumber*2 + 0], int1Mode, hint);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}

			hint.fields.interruptNumber = 1;
			rc = this->AssignEmbeddedHubLine(interruptMapping[portNumber*2 + 1], int2Mode, hint);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}
		} else {
			// Embedded HUB with interrupt controller

			// Interrupts are sequential for interrupt controller mapping, 2 interrupts for every port
			// so interrupt number will be port*2 + intNum
			// for HUBs with intController, the interruptMapping array is used to store endpoint number for every interrupt

			interruptMapping[portNumber*2 + 0] = deviceInfo.interrupt1Endpoint;
			interruptMapping[portNumber*2 + 1] = deviceInfo.interrupt2Endpoint;

			// INT 1
			rc = AssignInterruptControllerLine(portNumber*2 + 0, int1Mode);
			if (rc != EBF_OK) {
				return rc;
			}

			// INT 2
			rc = AssignInterruptControllerLine(portNumber*2 + 1, int2Mode);
			if (rc != EBF_OK) {
				return rc;
			}
		}
	} else {
		// Extender HUB. All extender HUBs have interrupt controller

		// Connect parent HUBs first, then open the interrupt controller port
		// HUBs are always on the first (0) endpoint
		// We need to pass current HUB device info, as any other device would do for the AssignInterruptLines call
		// The called function uses the interrupt lines mode, so we will prepare that structure for the call
		PnP_DeviceInfo hubDeviceInfo = {0};
		hubDeviceInfo.interrupt1Mode = this->hubInt1Mode;
		hubDeviceInfo.interrupt2Mode = this->hubInt2Mode;

		rc = pParentHub->AssignInterruptLines(parentPortNumber, 0, hubDeviceInfo);
		if (rc != EBF_OK) {
			EBF_REPORT_ERROR(rc);
			return rc;
		}

		// Interrupts are sequential for interrupt controller mapping, 2 interrupts for every port
		// so interrupt number will be port*2 + intNum
		// for HUBs with intController, the interruptMapping array is used to store endpoint number for every interrupt
		interruptMapping[portNumber*2 + 0] = deviceInfo.interrupt1Endpoint;
		interruptMapping[portNumber*2 + 1] = deviceInfo.interrupt2Endpoint;

		// INT 1
		rc = AssignInterruptControllerLine(portNumber*2 + 0, int1Mode);
		if (rc != EBF_OK) {
			return rc;
		}

		// INT 2
		rc = AssignInterruptControllerLine(portNumber*2 + 1, int2Mode);
		if (rc != EBF_OK) {
			return rc;
		}
	}

	return EBF_OK;
}

// We can't rely that Arduino's enumeration will not change some day
// So we'll have our own enumaration in the EEPROM and convert it to Arduino's equivalent when needed
uint8_t PnP_PlugAndPlayHub::GetArduinoInterruptMode(PnP_InterruptMode intMode)
{
	switch (intMode)
	{
	case PNP_NOT_CONECTED:
		// should not happen
		return (uint8_t)(-1);

	case PNP_INTERRUPT_ON_CHANGE:
		return CHANGE;

	case PNP_INTERRUPT_LOW:
		return LOW;

	case PNP_INTERRUPT_HIGH:
		return HIGH;

	case PNP_INTERRUPT_RISING:
		return RISING;

	case PNP_INTERRUPT_FALLING:
		return FALLING;

	default:
		// Should not happen
		return (uint8_t)(-1);
		break;
	}
}

uint8_t PnP_PlugAndPlayHub::Process()
{
	// HUBs should not get that call.
	// The Process() is called by the EBF_Logic directly for every registered HAL instance
	EBF_REPORT_ERROR(EBF_NOT_INITIALIZED);
	return EBF_NOT_INITIALIZED;

/*
	uint8_t rc;

	// Pass the Process call to all the devices. Connected HUBs will pass it even further.
	for (uint8_t i=0; i<numberOfPorts; i++) {
		if (pPortInfo[i].numberOfEndpoints > 0) {
			for (uint8_t j=0; j<pPortInfo[i].numberOfEndpoints; j++) {
				if (pPortInfo[i].pConnectedInstances[j] != 0) {
					rc = pPortInfo[i].pConnectedInstances[j]->Process();

					if (rc != EBF_OK) {
						EBF_REPORT_ERROR(rc);
						return rc;
					}
				}
			}
		}
	}

	return EBF_OK;
*/
}

void PnP_PlugAndPlayHub::ProcessInterrupt()
{
	uint8_t rc;
	InterruptHint hint;

	// HUB with interrupt controller
	if (intControllerChip.i2cAddress != 0) {
		uint16_t currentInputs;
		uint16_t changedLines;
		uint8_t stillProcessing = true;

		// The loop will continue while processing is still needed
		while (stillProcessing) {
			// clear the processing flag. it will be set on every processing pass, so another verification loop will be done
			stillProcessing = false;

        	// Get current inputs
        	rc = intControllerChip.GetInput(currentInputs);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return;
			}

        	// Process detected changes
        	changedLines = currentInputs ^ lastInputs;
			lastInputs = currentInputs;

			// Loop on all the bits to find what should be processed
			for (uint8_t i=0; i<16; i++) {
				// Determine if current change/mode should be processed as an interrupt based on the connected device interrupt mode
				switch (portInterruptMode[i]) {
					case PNP_INTERRUPT_ON_CHANGE:
						// Should process on any change
						if ((changedLines & 1<<i) != 0) {
							CallHalInterruptProcessing(i);

							stillProcessing = true;
						}
						break;

					case PNP_INTERRUPT_LOW:
						// Should process while currect value stays LOW
						if ((currentInputs & 1<<i) == 0) {
							CallHalInterruptProcessing(i);

							stillProcessing = true;
						}
						break;

					case PNP_INTERRUPT_HIGH:
						// Should process while currect value stays HIGH
						if ((currentInputs & 1<<i) != 0) {
							CallHalInterruptProcessing(i);

							stillProcessing = true;
						}
						break;

					default:
						// In general should not happen, but sometimes a second interrupt line
						// that is not connected changes due to cross-talk since the line is in the air
						break;
				}
			}

			// TODO: Optimization can be done
			// With 4/8 input modules there will be one more interrupt right after the processing loop exists.
			// The processing loop stats when INT line goes down. The additional ISR is called when the INT line goes up, and ignored.
			// We can detect that situation by reading the HUB interrupt line, but accessing extender HUB via I2C doesn't worth it.
			// If we can recognise that the HUB is connected to the MCU's GPIO directly, we could check if the interrupt line is low
			// on the MCU, which is almost 0 time.
		}
	} else {
		// Embedded HUB instance without interrupt controller
		EBF_Logic *pLogic = EBF_Logic::GetInstance();
		hint.uint32 = pLogic->GetInterruptHint();

		// ISR hint will tell us the port and endpoint that caused the interrupt
		CallHalInterruptProcessing(hint.fields.portNumber, hint.fields.endpointNumber);
	}
}

void PnP_PlugAndPlayHub::CallHalInterruptProcessing(uint8_t interruptNum)
{
	uint8_t portNumber;
	uint8_t endpointNumber;
	InterruptHint hint;

	// Every port might have 2 interrupts.
	// Since the mapping is sequential, we can divide by 2 to get the port number.
	portNumber = interruptNum >> 1;			// port is interrupt number divided by 2

	// For HUBs with interrupt controller, the interruptMapping array stores the endpoint number for every interrupt
	endpointNumber = interruptMapping[interruptNum];

	// Change the hint structure of the EBF_Logic with proper interrupt line, which is known only to the HUB with interrupt controller
	hint.fields.portNumber = portNumber;
	hint.fields.endpointNumber = endpointNumber;
	hint.fields.interruptNumber = interruptNum & 0x01;			// LSB of the index is the interrupt number 0 or 1

	EBF_Logic::GetInstance()->interruptHint = hint.uint32;

	// Pass it to the relevant HAL instance
	CallHalInterruptProcessing(portNumber, endpointNumber);
}

void PnP_PlugAndPlayHub::CallHalInterruptProcessing(uint8_t portNum, uint8_t endpointNum)
{
	if (pPortInfo[portNum].numberOfEndpoints > 0 &&
		pPortInfo[portNum].pConnectedInstances[endpointNum] != 0) {
		pPortInfo[portNum].pConnectedInstances[endpointNum]->ProcessInterrupt();
	}
}

// Switching to the specified port
// The pointer to the EBF_I2C is needed in order to support multiple I2C interfaces on the same controller
uint8_t PnP_PlugAndPlayHub::SwitchToPort(EBF_I2C* pPnpI2C, uint8_t portNumber)
{
	uint8_t rc;

	if (i2cSwitchChip.i2cAddress == 0) {
		// There is no switch for that HUB, just return OK
		return EBF_OK;
	} else {
		if (pParentHub != NULL) {
			// Switch parent HUBs first (from the main HUB up to this)
			rc = pParentHub->SwitchToPort(pPnpI2C, parentPortNumber);
			if (rc != EBF_OK) {
				EBF_REPORT_ERROR(rc);
				return rc;
			}
		}

		rc = i2cSwitchChip.Switch(portNumber);
		if (rc != EBF_OK) {
			EBF_REPORT_ERROR(rc);
			return rc;
		}
	}

	return EBF_OK;
}

// Setting an interrupt line is possible only for device that declared that line as a Digital Output
uint8_t PnP_PlugAndPlayHub::SetIntLine(uint8_t portNumber, uint8_t intLineNumber, uint8_t value)
{
	uint8_t rc;

	if (portNumber > maxPorts) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	if (portInterruptMode[portNumber*2 + intLineNumber] != PnP_InterruptMode::PNP_DIGITAL_OUTPUT) {
		EBF_REPORT_ERROR(EBF_INVALID_STATE);
		return EBF_INVALID_STATE;
	}

	if (intLineNumber > 1) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	// This is the main HUB without interrupt controller, the interrupt lines are directly connected to the MCU
	// interruptMapping holds the mapping of port/intLine to the interrupt line number
	if (intControllerChip.i2cAddress == 0) {
		// Set the corresponding line
		if (interruptMapping[portNumber*2 + intLineNumber] != (uint8_t)(-1)) {
			digitalWrite(interruptMapping[portNumber*2 + intLineNumber], value & 0x01);
		} else {
			EBF_REPORT_ERROR(EBF_NOT_INITIALIZED);
			return EBF_NOT_INITIALIZED;
		}
	} else {
		// For HUBs with interrupt controller the interrupt numbering is sequential for all the ports
		uint16_t currentValue;

		// Get current value
		rc = intControllerChip.GetOuput(currentValue);
		if (rc != EBF_OK) {
			return rc;
		}

		// Clear the bit for the output line
		currentValue &= ~(1<<(portNumber*2 + intLineNumber));

		// Set the bit
		if (value) {
			currentValue |= 1<<(portNumber*2 + intLineNumber);
		}

		// Set the line to the specified value
		rc = intControllerChip.SetOutput(currentValue);
		if (rc != EBF_OK) {
			return rc;
		}
	}

	return EBF_OK;
}

// Setting both interrupt lines is possible only for device that declared those line as a Digital Outputs
uint8_t PnP_PlugAndPlayHub::SetIntLinesValue(uint8_t portNumber, uint8_t value)
{
	uint8_t rc;

	if (portNumber > maxPorts) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	if (portInterruptMode[portNumber*2 + 0] != PnP_InterruptMode::PNP_DIGITAL_OUTPUT) {
		EBF_REPORT_ERROR(EBF_INVALID_STATE);
		return EBF_INVALID_STATE;
	}

	if (portInterruptMode[portNumber*2 + 1] != PnP_InterruptMode::PNP_DIGITAL_OUTPUT) {
		EBF_REPORT_ERROR(EBF_INVALID_STATE);
		return EBF_INVALID_STATE;
	}

	// This is the main HUB without interrupt controller, the interrupt lines are directly connected to the MCU
	// interruptMapping holds the mapping of port/intLine to the interrupt line number
	if (intControllerChip.i2cAddress == 0) {
		// Set the corresponding lines
		if (interruptMapping[portNumber*2 + 0] != (uint8_t)(-1)) {
			digitalWrite(interruptMapping[portNumber*2 + 0], value & 0x01);
		}

		if (interruptMapping[portNumber*2 + 1] != (uint8_t)(-1)) {
			digitalWrite(interruptMapping[portNumber*2 + 1], value & 0x02);
		}
	} else {
		// For HUBs with interrupt controller the interrupt numbering is sequential for all the ports
		uint16_t currentValue;

		// Get current value
		rc = intControllerChip.GetOuput(currentValue);
		if (rc != EBF_OK) {
			return rc;
		}

		// Clear the bits for the output lines
		currentValue &= ~(1<<(portNumber*2 + 0));
		currentValue &= ~(1<<(portNumber*2 + 1));

		// Set the bits
		if (value & 0x01) {
			currentValue |= 1<<(portNumber*2 + 0);
		}
		if (value & 0x02) {
			currentValue |= 1<<(portNumber*2 + 1);
		}

		// Set the line to the specified value
		rc = intControllerChip.SetOutput(currentValue);
		if (rc != EBF_OK) {
			return rc;
		}
	}

	return EBF_OK;
}

// Getting both interrupt lines values
uint8_t PnP_PlugAndPlayHub::GetIntLinesValue(uint8_t portNumber, uint8_t &value)
{
	uint8_t rc;

	value = 0;

	if (portNumber > maxPorts) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	// This is the main HUB without interrupt controller, the interrupt lines are directly connected to the MCU
	// interruptMapping holds the mapping of port/intLine to the interrupt line number
	if (intControllerChip.i2cAddress == 0) {
		// Get the corresponding lines values
		if (interruptMapping[portNumber*2 + 0] != (uint8_t)(-1)) {
			value |= digitalRead(interruptMapping[portNumber*2 + 0]) & 0x01;
		}

		if (interruptMapping[portNumber*2 + 1] != (uint8_t)(-1)) {
			value |= ((digitalRead(interruptMapping[portNumber*2 + 1]) & 0x01) << 1);
		}
	} else {
		// For HUBs with interrupt controller the interrupt numbering is sequential for all the ports
		uint16_t currentValue;

		// Get current value
		rc = intControllerChip.GetInput(currentValue);
		if (rc != EBF_OK) {
			return rc;
		}

		if(currentValue & (1<<(portNumber*2 + 0))) {
			value |= 0x01;
		}

		if(currentValue & (1<<(portNumber*2 + 1))) {
			value |= 0x02;
		}
	}

	return EBF_OK;
}

uint8_t PnP_PlugAndPlayHub::GetIntLine(uint8_t portNumber, uint8_t intLineNumber, uint8_t &value)
{
	uint8_t rc;
	value = 0;

	if (portNumber > maxPorts) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	if (intLineNumber > 1) {
		EBF_REPORT_ERROR(EBF_INDEX_OUT_OF_BOUNDS);
		return EBF_INDEX_OUT_OF_BOUNDS;
	}

	// This is the main HUB without interrupt controller, the interrupt lines are directly connected to the MCU
	// interruptMapping holds the mapping of port/intLine to the interrupt line number
	if (intControllerChip.i2cAddress == 0) {
		// Get the corresponding line
		if (interruptMapping[portNumber*2 + intLineNumber] != (uint8_t)(-1)) {
			value |= digitalRead(interruptMapping[portNumber*2 + intLineNumber]);
		} else {
			EBF_REPORT_ERROR(EBF_NOT_INITIALIZED);
			return EBF_NOT_INITIALIZED;
		}
	} else {
		// For HUBs with interrupt controller the interrupt numbering is sequential for all the ports
		uint16_t currentValue;

		// Get current value
		rc = intControllerChip.GetInput(currentValue);
		if (rc != EBF_OK) {
			return rc;
		}

		// PnP is working with reverse logic, "1" logic = GND, "0" logic = HIGH
		if(currentValue & (1<<(portNumber*2 + intLineNumber))) {
			value = 1;
		}
	}

	return EBF_OK;
}
