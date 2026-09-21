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

		// The SetValue and GetValue are inherited from the PnP_OutputInterface class

	protected:
		uint8_t IsProcessingNeeded() { return 0; }
		uint8_t Process() { return EBF_OK; }
};

#endif
