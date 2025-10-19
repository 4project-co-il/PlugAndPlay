#ifndef __PNP_BASIC_INPUT_INTERFACE_H__
#define __PNP_BASIC_INPUT_INTERFACE_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "PnP_InputInterface.h"

class PnP_BasicInputInterface : public PnP_InputInterface {
	public:
		PnP_BasicInputInterface();

		virtual InputInterface_Type GetType() { return InputInterface_Type::INPUT; }

		uint8_t SetOnChange(EBF_CallbackType callbackFunc) { this->onChangeCallback = callbackFunc; return EBF_OK; }

	protected:
		EBF_CallbackType onChangeCallback;

		virtual void ExecuteCallback() { onChangeCallback(); }
	};

#endif
