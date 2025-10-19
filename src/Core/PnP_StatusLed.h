#ifndef __PNP_STATUS_LED_H__
#define __PNP_STATUS_LED_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Products/EBF_Led.h"

// PnP_StatusLed class wraps the EBF_Led implementation, hiding the initialization pin
// Current default implementation is to use LED_BUILTIN pin
class PnP_StatusLed : public EBF_Led {
	public:
		PnP_StatusLed();

		uint8_t Init();

};

#endif
