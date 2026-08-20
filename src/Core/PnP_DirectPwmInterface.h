#ifndef __PNP_DIRECT_PWM_INTERFACE_H__
#define __PNP_DIRECT_PWM_INTERFACE_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "PnP_OutputInterface.h"

class PnP_DirectPwmInterface : public PnP_OutputInterface {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_DirectPwmInterface");

	public:
		PnP_DirectPwmInterface() {}

		virtual OutputInterface_Type GetType() { return OutputInterface_Type::DIRECT_PWM; }

		// Sets PWM timing value
		// float percent paramenter: 0=Complete OFF, 50=50% width ON/OFF, 100=Complete ON
		uint8_t SetValue(float value);
		uint8_t GetValue();

	protected:
		uint8_t IsProcessingNeeded() { return 0; }
		uint8_t Process() { return EBF_OK; }
};

#endif
