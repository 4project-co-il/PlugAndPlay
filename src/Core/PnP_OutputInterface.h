#ifndef __PNP_OUTPUT_INTERFACE_H__
#define __PNP_OUTPUT_INTERFACE_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "PnP_OutputInterfaceProvider.h"
#include "../Modules/PnP_Module_1SimpleLed.h"

class PnP_OutputInterface {
	public:
		friend class PnP_Module_1SimpleLed;
		friend class PnP_Module_2SimpleLeds;

		PnP_OutputInterface();

		enum OutputInterface_Type : uint8_t {
			DIRECT_PWM = 0,		// Direct PWM control
			SIMPLE_LED,			// Simple led can only be turned ON or OFF
			ADVANCED_LED,		// Advanced led have brightness control
			RELAY,				// Relay can be turned ON or OFF
		};

		virtual OutputInterface_Type GetType() = 0;

		uint8_t SetValue(float value) { return pOutputProvider->SetValue_OIP(providerIndex, value); }
		float GetValue() { return pOutputProvider->GetValue_OIP(providerIndex); }

	protected:
		PnP_OutputInterfaceProvider* pOutputProvider;
		uint8_t providerIndex;

		virtual uint8_t AssignInterfaceProvider(PnP_OutputInterfaceProvider* pProvider, uint8_t index);
		virtual uint8_t IsProcessingNeeded() { return 0; }
		virtual uint8_t Process() { return EBF_OK; };
		virtual uint8_t SetInitialValue(uint8_t value) { return EBF_OK; }

		// Current output provider that generates the callbacks
		static PnP_OutputInterfaceProvider* pCurrentProvider;
};

#endif
