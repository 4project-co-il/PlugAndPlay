#include "PnP_SimpleLedInterface.h"

PnP_SimpleLedInterface::PnP_SimpleLedInterface()
{
	state = LED_OFF;
}

// SetValue acts as an ON/OFF function, value == 0 will perform as OFF, any other value as ON
uint8_t PnP_SimpleLedInterface::SetValue(uint8_t value)
{
	uint8_t rc;

	if (value == 0) {
		state = LED_OFF;

		rc = pOutputProvider->SetValue_OIP(providerIndex, 0.0);
	} else {
		state = LED_ON;

		rc = pOutputProvider->SetValue_OIP(providerIndex, 100.0);
	}

	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
	}

	return rc;
}

// GetValue returns current status of the led (ON or OFF)
float PnP_SimpleLedInterface::GetValue()
{
	switch (state)
	{
		case LED_OFF:
		case LED_BLINKING_OFF:
			return 0.0;

		case LED_ON:
		case LED_BLINKING_ON:

			return 100.0;
	}

	return 0;
}

uint8_t PnP_SimpleLedInterface::On()
{
	state = LED_ON;

	return pOutputProvider->SetValue_OIP(providerIndex, 100.0);
}

uint8_t PnP_SimpleLedInterface::Off()
{
	state = LED_OFF;

	return pOutputProvider->SetValue_OIP(providerIndex, 0.0);
}

// Turns on for msOn milliSeconds and stay off for msOff milliSeconds
uint8_t PnP_SimpleLedInterface::Blink(uint16_t msOn, uint16_t msOff)
{
	uint8_t rc;

	onDuration = msOn;
	offDuration = msOff;

	// Start from the BLINKING_ON state
	state = LED_BLINKING_ON;
	effectStart = pOutputProvider->micros_OIP();

	// Force processing, it will recalculate the needed polling interval there
	pOutputProvider->SetPollingInterval_OIP(0);

	rc = On();
	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}

uint8_t PnP_SimpleLedInterface::IsProcessingNeeded()
{
	switch (state)
	{
	case LED_ON:
	case LED_OFF:
		// Nothing to do
		// No polling needed
		return 0;
		break;

	case LED_BLINKING_ON:
	case LED_BLINKING_OFF:
		// Should follow the timing
		return 1;
		break;
	}

	// Should not get here...
	EBF_REPORT_ERROR(EBF_INVALID_STATE);
	return 0;
}

uint8_t PnP_SimpleLedInterface::Process()
{
	uint8_t rc = EBF_OK;
	unsigned long timePassed;

	switch (state)
	{
	case LED_ON:
	case LED_OFF:
		// Nothing to do
		// No polling needed
		pOutputProvider->SetPollingInterval_OIP(EBF_NO_POLLING);
		break;

	case LED_BLINKING_ON:
		timePassed = pOutputProvider->micros_OIP() - effectStart;

		if (timePassed > onDuration * 1000) {
			// On duration passed, turn the led off and set the data for BLINKING_OFF state
			rc = Off();

			state = LED_BLINKING_OFF;
			effectStart = pOutputProvider->micros_OIP();

			// next polling is the duration of the OFF state
			pOutputProvider->SetPollingInterval_OIP(offDuration);
		} else {
			// On duration didn't pass yet, processing was called before needed
			// Set polling to the time left
			pOutputProvider->SetPollingInterval_OIP(onDuration - timePassed / 1000);
		}
		break;

	case LED_BLINKING_OFF:
		timePassed = pOutputProvider->micros_OIP() - effectStart;

		if (timePassed > offDuration * 1000) {
			// Off duration passed, turn the led on and set the data for BLINKING_ON state
			rc = On();

			state = LED_BLINKING_ON;
			effectStart = pOutputProvider->micros_OIP();

			// next polling is the duration of the OFF state
			pOutputProvider->SetPollingInterval_OIP(onDuration);
		} else {
			// On duration didn't pass yet, processing was called before needed
			// Set polling to the time left
			pOutputProvider->SetPollingInterval_OIP(offDuration - timePassed / 1000);
		}
		break;

	default:
		rc = EBF_INVALID_STATE;
		break;
	}

	if (rc != EBF_OK) {
		EBF_REPORT_ERROR(rc);
		return rc;
	}

	return EBF_OK;
}
