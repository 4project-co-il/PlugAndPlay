#ifndef _PNP_UART_H__
#define _PNP_UART_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Core/EBF_Serial.h"

// PnP_UART class wraps the EBF_Serial implementation, using the Serial1 interface
// Default settings are 115200 bps, 8N1
class PnP_UART : public EBF_Serial {
	public:
		PnP_UART();

		uint8_t Init(uint32_t boudRate = 115200, uint16_t config = SERIAL_8N1);
};

#endif
