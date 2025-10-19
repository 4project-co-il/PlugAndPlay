#ifndef _PNP_MODULE_1INPUT_H__
#define _PNP_MODULE_1INPUT_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../Core/PnP_PlugAndPlayDevice.h"
#include "../Core/PnP_PlugAndPlayI2C.h"

class PnP_Module_1Input : protected EBF_HalInstance {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_Module_1Input");

	public:
		PnP_Module_1Input();

		uint8_t Init();

		uint8_t GetValue();

		void SetOnChange(EBF_CallbackType onChangeCallback) { this->onChangeCallback = onChangeCallback; }

		uint8_t PostponeProcessing();
		uint8_t InInterrupt() {
			EBF_Logic *pLogic = EBF_Logic::GetInstance();
			return pLogic->IsRunFromIsr();
		}

	private:
		uint8_t Process();
		void ProcessInterrupt();

	 	uint8_t GetIntLine(uint8_t line, uint8_t &value);

	private:
		PnP_PlugAndPlayI2C *pPnPI2C;

		// Callbacks
		EBF_CallbackType onChangeCallback;
};

#endif
