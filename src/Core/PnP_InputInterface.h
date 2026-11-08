#ifndef __PNP_INPUT_INTERFACE_H__
#define __PNP_INPUT_INTERFACE_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "PnP_InputInterfaceProvider.h"

class PnP_InputInterface {
	public:
		friend class PnP_Module_2Inputs;
		friend class PnP_Module_4Inputs;
		friend class PnP_Module_8Inputs;

		PnP_InputInterface();

		enum InputInterface_Type : uint8_t {
			INPUT = 0,
			SWITCH,
			BUTTON,
			ENCODER,
		};

		virtual InputInterface_Type GetType() = 0;

		virtual uint8_t GetValue() { return pInputProvider->GetValue_IIP(providerIndex); }
		virtual uint8_t GetLastValue() { return pInputProvider->GetLastValue_IIP(providerIndex); }

		void SetUserData(uint32_t data) { this->userData = data; }
		uint32_t GetUserData() { return this->userData; }

		static PnP_InputInterface* GetCurrentInterface() { return pCurrentProvider->GetCurrentInterface(); }

		virtual uint8_t AssignInterfaceProvider(PnP_InputInterfaceProvider* pProvider, uint8_t index);

		void SetPollingInterval(uint32_t ms);
		uint32_t GetPollingInterval() { return pollingInterval; }
		unsigned long millis() { return pInputProvider->millis_IIP(); }
		unsigned long micros() { return pInputProvider->micros_IIP(); }

	private:
		PnP_InputInterfaceProvider* pInputProvider;
		uint8_t providerIndex;

		virtual void ExecuteCallback() = 0;
		virtual uint8_t Process() { return EBF_OK; };
		virtual uint8_t SetInitialValue(uint8_t value) { return EBF_OK; }

		// Save requested polling interval for every interface, since input provider will query it
		uint32_t pollingInterval;

		// User data
		uint32_t userData;

		// Current input provider that generates the callbacks
		static PnP_InputInterfaceProvider* pCurrentProvider;
};

#endif
