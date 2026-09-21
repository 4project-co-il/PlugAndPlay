#include "PnP_ServoMotorInterface.h"


// Sets servo motor position
// float percent paramenter: 0=Left-most position, 50=Middle position, 100=Right-most position
uint8_t PnP_ServoMotorInterface::SetValue(float value)
{
	if (value < 0.0) value = 0.0;
	if (value > 100.0) value = 100.0;

	// Calculate the position after scaling
	value = mapf(value, 0.0, 100.0, minBoundary, maxBoundary);

	return pOutputProvider->SetValue_OIP(providerIndex, value);
}

// GetValue returns current status of the led (ON or OFF)
float PnP_ServoMotorInterface::GetValue()
{
	// Get current position from the provider and scale it back
	float value = pOutputProvider->GetValue_OIP(providerIndex);

	return mapf(value, minBoundary, maxBoundary, 0.0, 100.0);
}

uint8_t PnP_ServoMotorInterface::SetScaleBoundaries(float minMs, float maxMs)
{
	float updatePeriod = 1000 / pOutputProvider->GetUpdateFrequency(providerIndex);

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