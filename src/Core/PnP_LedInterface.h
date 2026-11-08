#ifndef __PNP_LED_INTERFACE_H__
#define __PNP_LED_INTERFACE_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "PnP_OutputInterface.h"

class PnP_LedInterface : public PnP_OutputInterface {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_LedInterface");

	public:
		PnP_LedInterface();

		virtual OutputInterface_Type GetType() { return OutputInterface_Type::LED; }

		uint8_t On();
		uint8_t Off();

		uint8_t SetBrightness(float percent);
		float GetBrightness() { return brightness; }

		uint8_t Blink(uint16_t msOn, uint16_t msOff);

		uint8_t FadeIn(uint16_t msDuration, uint8_t msUpdate = 20);
		uint8_t FadeOut(uint16_t msDuration, uint8_t msUpdate = 20);

		uint8_t SetValue(uint8_t value);
		uint8_t SetValue(float value) { return this->SetValue((uint8_t)value); }	// PnP_OutputInterface override
		float GetValue();

	protected:
		uint8_t Process();

	protected:
		enum LedState : uint8_t {
			LED_OFF = 0,
			LED_ON,
			LED_BLINKING_ON,
			LED_BLINKING_OFF,
			LED_FADING_IN,
			LED_FADING_OUT,
		};

		LedState state;
		float brightness;			// 0%-100%
		unsigned long effectStart;	// in micro-Sec
		uint16_t onDuration;		// in milli-Sec, duration of the fading effect during fading
		uint16_t offDuration;		// in milli-Sec
		uint8_t fadeUpdatePeriod;	// in milli-Sec

};

#endif
