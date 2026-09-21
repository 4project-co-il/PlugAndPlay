#ifndef __PNP_SERVO_MOTOR_INTERFACE_H__
#define __PNP_SERVO_MOTOR_INTERFACE_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "PnP_OutputInterface.h"

class PnP_ServoMotorInterface : public PnP_OutputInterface {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_ServoMotorInterface");

	public:
		PnP_ServoMotorInterface() {};

		virtual OutputInterface_Type GetType() { return OutputInterface_Type::SERVO_MOTOR; }

		// Sets servo motor position
		// float percent paramenter: 0=Left-most position, 50=Middle position, 100=Right-most position
		uint8_t SetValue(float value);
		float GetValue();

		// Generic servo motors expect to receive a pulse width between 1 and 2 mSec
		// while 1mSec is the left-most position, 1.5mSec is the center and 2mSec is the right-most position
		uint8_t SetScaleBoundaries(float minMs = 1.0, float maxMs = 2.0);

	protected:
		uint8_t IsProcessingNeeded() { return 0; }
		uint8_t Process() { return EBF_OK; }

		float minBoundary;
		float maxBoundary;

		float mapf(float x, float in_min, float in_max, float out_min, float out_max);
};

#endif
