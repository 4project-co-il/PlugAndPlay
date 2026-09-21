#ifndef __PNP_SIMPLE_LED_INTERFACE_H__
#define __PNP_SIMPLE_LED_INTERFACE_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "PnP_OutputInterface.h"

class PnP_SimpleLedInterface : public PnP_OutputInterface {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_SimpleLedInterface");

	public:
		PnP_SimpleLedInterface();

		virtual OutputInterface_Type GetType() { return OutputInterface_Type::SIMPLE_LED; }

		uint8_t On();
		uint8_t Off();

		uint8_t Blink(uint16_t msOn, uint16_t msOff);

		uint8_t SetValue(uint8_t value);
		uint8_t SetValue(float value) { return this->SetValue((uint8_t)value); }	// PnP_OutputInterface override
		float GetValue();

	protected:
		uint8_t IsProcessingNeeded();
		uint8_t Process();

	private:
		enum LedState : uint8_t {
			LED_OFF = 0,
			LED_ON,
			LED_BLINKING_ON,
			LED_BLINKING_OFF,
		};

		LedState state;
		uint16_t onDuration;		// in milli-Sec
		uint16_t offDuration;		// in milli-Sec
		unsigned long effectStart;	// in micro-Sec
};

#endif
