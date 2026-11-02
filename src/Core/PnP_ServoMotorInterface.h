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
		PnP_ServoMotorInterface();

		virtual OutputInterface_Type GetType() { return OutputInterface_Type::SERVO_MOTOR; }

		// Sets servo motor position
		// float percent paramenter: 0=Left-most position, 50=Middle position, 100=Right-most position
		// msDuration can be set to a time in mSec that should take the movement to be complete
		// the msUpdate parameter specifies the update period in mSec to smooth the movement
		uint8_t SetValue(float value, uint16_t msDuration = 0, uint8_t msUpdate = 10);

		// Gets current value of the servo (from 0% to 100%)
		float GetValue();

		// Generic servo motors expect to receive a pulse width between 1mSec and 2mSec range
		// While most of the motors those days expect ranges from 0.5mSec to 2.5mSec
		// while 0.5/1mSec is the left-most position, 1.5mSec is the center and 2/2.5mSec is the right-most position
		uint8_t SetScaleBoundaries(float minMs = 0.5, float maxMs = 2.5);

	protected:
		uint8_t IsProcessingNeeded() { return 0; }
		uint8_t Process();
		uint8_t AssignInterfaceProvider(PnP_OutputInterfaceProvider* pProvider, uint8_t index);

		float minBoundary;
		float maxBoundary;

		float mapf(float x, float in_min, float in_max, float out_min, float out_max);

		// Parameters for interpolated movement
		float lastValue;			// Last value of the servo position
		float stepDelta;			// How much should be added to servo's value on every step
		float targetValue;			// The value were the server should arrive to
		unsigned long effectStart;	// in micro-Sec
		uint8_t updatePeriod;		// in milli-Sec

};

#endif
