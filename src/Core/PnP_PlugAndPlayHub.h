#ifndef __PNP_PLUGANDPLAY_HUB_H__
#define __PNP_PLUGANDPLAY_HUB_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Core/EBF_I2C.h"
#include "PnP_PlugAndPlayDevice.h"
#include "PnP_PlugAndPlayManager.h"
#include "PnP_PlugAndPlayI2C.h"
#include "../../../EventBasedFramework/src/HAL/EBF_HAL_TCAL9539.h"
#include "../../../EventBasedFramework/src/HAL/EBF_HAL_PCA9548.h"

class PnP_PlugAndPlayManager;
class PnP_PlugAndPlayI2C;

class PnP_PlugAndPlayHub : protected EBF_HalInstance {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_PlugAndPlayHub");

	public:
		friend class PnP_PlugAndPlayManager;

		static const uint8_t maxPorts = 8;

		PnP_PlugAndPlayHub(EBF_I2C* pI2C);

		uint8_t Init(PnP_PlugAndPlayHub *pParentHub, uint8_t parentPort, PnP_DeviceInfo &deviceInfo, uint8_t *pParams);
		uint8_t SwitchToPort(EBF_I2C* pPnpI2C, uint8_t portNumber);

		// Setting an interrupt line is possible only for device that declared that line as a Digital Output
		uint8_t SetIntLine(uint8_t portNumber, uint8_t intLineNumber, uint8_t value);
		uint8_t SetIntLinesValue(uint8_t portNumber, uint8_t value);
		uint8_t GetIntLine(uint8_t portNumber, uint8_t intLineNumber, uint8_t &value);
		uint8_t GetIntLinesValue(uint8_t portNumber, uint8_t &value);

		typedef union {
			struct {
				uint32_t interruptNumber : 1;
				uint32_t portNumber : 4;
				uint32_t endpointNumber : 4;
				uint32_t reserved : 22;
				// This bit will be set to 1 when an interrupt is attached and will be used as a flag to know when we already attached it
				uint32_t attached : 1;
			} fields;
			uint32_t uint32;
		} InterruptHint;

		uint8_t AssignInterruptLines(uint8_t portNumber, uint8_t endpointNumber, PnP_DeviceInfo &deviceInfo);

		typedef struct {
			EBF_HalInstance** pConnectedInstances;	// Array of HalInstance pointers assigned to the device connected to that port
			uint8_t numberOfEndpoints;				// Number of endpoints on the device connected to that port
		} PortInfo;

	protected:
		uint8_t Process();
		void ProcessInterrupt();

		// Calls attached HAL instance interrupt processing
		void CallHalInterruptProcessing(uint8_t portNum, uint8_t endpointNum);
		// Calls attached HAL instance interrupt processing for HUB with interrupt controller
		void CallHalInterruptProcessing(uint8_t interruptNum);

		uint8_t AssignEmbeddedHubLine(uint8_t pinNumber, PnP_InterruptMode intMode, InterruptHint intHint);
		uint8_t AssignInterruptControllerLine(uint8_t intLine, PnP_InterruptMode intMode);

		uint8_t GetArduinoInterruptMode(PnP_InterruptMode intMode);

		PnP_PlugAndPlayHub* pParentHub;
		uint8_t parentPortNumber;
		uint8_t numberOfPorts;
		PortInfo* pPortInfo;
		uint8_t routingLevel;

		// For embedded HUBs - stores the interrupt line number on the controller, 2 interrupts for each port
		// For interrupt controller HUB - stores endpoint number for each interrupt
		uint8_t interruptMapping[maxPorts * 2];

		// Save HUB's interrupt modes, needed for extender assignment
		PnP_InterruptMode hubInt1Mode;
		PnP_InterruptMode hubInt2Mode;

		// Interrupt mode for every port
		PnP_InterruptMode portInterruptMode[maxPorts * 2];

		// HAL chips
		EBF_HAL_TCAL9539 intControllerChip;
		EBF_HAL_PCA9548 i2cSwitchChip;

		uint16_t lastInputs;
};

#endif
