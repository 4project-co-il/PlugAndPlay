#include "PnP_ServoMotorInterface.h"

PnP_ServoMotorInterface::PnP_ServoMotorInterface()
{
	lastValue = 0.0;
	targetValue = 0.0;
	stepDelta = 0.0;
	effectStart = 0;
	updatePeriod = 0;
}

// Sets servo motor position
// float percent paramenter: 0=Left-most position, 50=Middle position, 100=Right-most position
uint8_t PnP_ServoMotorInterface::SetValue(float value, uint16_t msDuration, uint8_t msUpdate)
{
	if (value < 0.0) value = 0.0;
	if (value > 100.0) value = 100.0;

	if (msDuration == 0) {
		// No interpolation needed, immediate execution

		// Stop the interpolation if it was running
		if (updatePeriod != 0) {
			updatePeriod = 0;
			PnP_OutputInterface::SetPollingInterval(EBF_NO_POLLING);
		}
	} else {
		// Interpolation requested, save the parameters
		targetValue = value;
		effectStart = micros();
		lastValue = GetValue();
		updatePeriod = msUpdate;

		// Calculate step size for every update
		stepDelta = ((targetValue - lastValue) / msDuration) * msUpdate;

		// Start with the first movement
		lastValue += stepDelta;

		// We might need to do only 1 update
		if (updatePeriod >= msDuration) {
			// Check if we've reached the target
			if (stepDelta > 0.0) {
				if (lastValue >= targetValue) {
					updatePeriod = 0;
				}
			} else {
				if (lastValue <= targetValue) {
					updatePeriod = 0;
				}
			}
		}

		if (updatePeriod == 0) {
			// No update needed
			PnP_OutputInterface::SetPollingInterval(EBF_NO_POLLING);
		} else {
			// Requesting Process() call every update period
			PnP_OutputInterface::SetPollingInterval(updatePeriod);
		}

		value = lastValue;
	}

	// Calculate the position after scaling
	value = mapf(value, 0.0, 100.0, minBoundary, maxBoundary);

	return PnP_OutputInterface::SetValue(value);
}

// GetValue returns current status of the led (ON or OFF)
float PnP_ServoMotorInterface::GetValue()
{
	// Get current position from the provider and scale it back
	float value = PnP_OutputInterface::GetValue();

	return mapf(value, minBoundary, maxBoundary, 0.0, 100.0);
}

uint8_t PnP_ServoMotorInterface::SetScaleBoundaries(float minMs, float maxMs)
{
	float updatePeriod = 1000.0 / PnP_OutputInterface::GetUpdateFrequency();

	if (minMs < 0.0) minMs = 0.0;
	if (minMs > updatePeriod) minMs = updatePeriod;

	if (maxMs < 0.0) maxMs = 0.0;
	if (maxMs > updatePeriod) maxMs = updatePeriod;

	// Save the boundaries. Will be used in Set and Get functions
	minBoundary = minMs * 100.0 / updatePeriod;
	maxBoundary = maxMs * 100.0 / updatePeriod;

	return EBF_OK;
}

float PnP_ServoMotorInterface::mapf(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint8_t PnP_ServoMotorInterface::AssignInterfaceProvider(PnP_OutputInterfaceProvider* pProvider, uint8_t index)
{
	uint8_t rc;

	rc = PnP_OutputInterface::AssignInterfaceProvider(pProvider, index);
	if (rc != EBF_OK) {
		EBF_REPORT_AND_RETURN(rc);
	}

	SetScaleBoundaries();

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_ServoMotorInterface::Process()
{
	uint8_t rc = EBF_OK;
	unsigned long timePassed;
	float setValue;

	// No update is needed
	if (updatePeriod == 0) {
		return EBF_OK;
	}

	timePassed = PnP_OutputInterface::micros() - effectStart;

	if (timePassed > updatePeriod * 1000) {
		// Next movement step
		lastValue += stepDelta;

		// Check if we've reached the target
		if (stepDelta > 0.0) {
			if (lastValue >= targetValue) {
				updatePeriod = 0;
			}
		} else {
			if (lastValue <= targetValue) {
				updatePeriod = 0;
			}
		}

		// Calculate the position after scaling
		setValue = mapf(lastValue, 0.0, 100.0, minBoundary, maxBoundary);

		rc = PnP_OutputInterface::SetValue(setValue);

		if (updatePeriod == 0) {
			// Reached the target, no more processing is needed
			PnP_OutputInterface::SetPollingInterval(EBF_NO_POLLING);
		} else {
			// Request next processing after the update period
			PnP_OutputInterface::SetPollingInterval(updatePeriod);
		}
	} else {
		// Update period didn't pass yet, processing was called before needed
		// Set polling to the time left
		PnP_OutputInterface::SetPollingInterval(updatePeriod - timePassed / 1000);
	}

	EBF_REPORT_AND_RETURN(rc);
}
