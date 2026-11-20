#include "PnP_LedInterface.h"

PnP_LedInterface::PnP_LedInterface()
{
	state = LED_OFF;
	// Full brightness by default
	brightness = 100.0;
}

// SetValue acts as an ON/OFF function, value == 0 will perform as OFF, any other value as ON
uint8_t PnP_LedInterface::SetValue(float value)
{
	uint8_t rc;

	if (value == 0) {
		state = LED_OFF;
		rc = PnP_OutputInterface::SetValue(0.0);
	} else {
		state = LED_ON;

		rc = PnP_OutputInterface::SetValue(brightness);
	}

	EBF_REPORT_AND_RETURN(rc);
}

// GetValue returns 0 when the LED is OFF, or brightness level when the LED is ON
float PnP_LedInterface::GetValue()
{
	switch (state)
	{
		case LED_OFF:
		case LED_BLINKING_OFF:
			return 0.0;

		case LED_ON:
		case LED_BLINKING_ON:
		case LED_FADING_IN:
		case LED_FADING_OUT:
			return brightness;
	}

	return 0.0;
}

uint8_t PnP_LedInterface::On()
{
	uint8_t rc;

	state = LED_ON;

	rc = PnP_OutputInterface::SetValue(brightness);

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_LedInterface::Off()
{
	uint8_t rc;

	state = LED_OFF;

	rc = PnP_OutputInterface::SetValue(0.0);

	EBF_REPORT_AND_RETURN(rc);
}

// Brightness will work only on hardware PWM enabled pins
uint8_t PnP_LedInterface::SetBrightness(float percent)
{
	uint8_t rc = EBF_OK;

	if (percent < 0.0) {
		percent = 0.0;
	}
	if (percent > 100.0) {
		percent = 100.0;
	}

	brightness = percent;

	switch (state)
	{
	case LED_OFF:
	case LED_BLINKING_OFF:
	case LED_FADING_IN:
	case LED_FADING_OUT:
		// do nothing
		break;

	case LED_ON:
	case LED_BLINKING_ON:
		// Update the brightness
		rc = PnP_OutputInterface::SetValue(brightness);
		break;

	default:
		rc = EBF_INVALID_STATE;
		break;
	}

	EBF_REPORT_AND_RETURN(rc);
}

// Turns on for msOn milliSeconds and stay off for msOff milliSeconds, for the repeatTimes times
uint8_t PnP_LedInterface::Blink(uint16_t msOn, uint16_t msOff, uint8_t repeatTimes)
{
	uint8_t rc;

	onDuration_FadingDuration = msOn;
	offDuration = msOff;
	fadeUpdatePeriod_repeatTimes = repeatTimes-1;

	// Start from the BLINKING_ON state
	state = LED_BLINKING_ON;
	effectStart = PnP_OutputInterface::micros();

	// Force processing, it will recalculate the needed polling interval there
	PnP_OutputInterface::SetPollingInterval(0);

	rc = PnP_OutputInterface::SetValue(brightness);

	EBF_REPORT_AND_RETURN(rc);
}

// The led will fade in, from OFF to ON (up to brightness)
// The process will last msDuration milli-seconds
// and the value will be updated every msUpdate milli-seconds, 20mSec by default
uint8_t PnP_LedInterface::FadeIn(uint16_t msDuration, uint8_t msUpdate)
{
	uint8_t rc;

	// Save the update period and set the polling interval accordingly
	fadeUpdatePeriod_repeatTimes = msUpdate;
	PnP_OutputInterface::SetPollingInterval(fadeUpdatePeriod_repeatTimes);

	// Save total fading duration
	onDuration_FadingDuration = msDuration;

	effectStart = PnP_OutputInterface::micros();
	state = LED_FADING_IN;

	rc = PnP_OutputInterface::SetValue(0.0);

	EBF_REPORT_AND_RETURN(rc);
}

// The led will fade out, from ON (current brightness) to OFF
// The process will last msDuration milli-seconds
// and the value will be updated every msUpdate milli-seconds, 20mSec by default
uint8_t PnP_LedInterface::FadeOut(uint16_t msDuration, uint8_t msUpdate)
{
	uint8_t rc;

	// Save the update period and set the polling interval accordingly
	fadeUpdatePeriod_repeatTimes = msUpdate;
	PnP_OutputInterface::SetPollingInterval(fadeUpdatePeriod_repeatTimes);

	// Save total fading duration
	onDuration_FadingDuration = msDuration;

	effectStart = PnP_OutputInterface::micros();
	state = LED_FADING_OUT;

	rc = PnP_OutputInterface::SetValue(brightness);

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_LedInterface::Process()
{
	uint8_t rc = EBF_OK;
	unsigned long timePassed;
	uint16_t fadingBrightness;

	switch (state)
	{
	case LED_ON:
	case LED_OFF:
		// Nothing to do
		// No polling needed
		PnP_OutputInterface::SetPollingInterval(EBF_NO_POLLING);
		break;

	case LED_BLINKING_ON:
		timePassed = PnP_OutputInterface::micros() - effectStart;

		if (timePassed > onDuration_FadingDuration * 1000) {
			// On duration passed, turn the led off and set the data for BLINKING_OFF state
			rc = PnP_OutputInterface::SetValue(0.0);

			// No more repeatitions for the blink
			if (fadeUpdatePeriod_repeatTimes == 0) {
				// Move to OFF state
				state = LED_OFF;

				PnP_OutputInterface::SetPollingInterval(EBF_NO_POLLING);
			} else {
				state = LED_BLINKING_OFF;
				effectStart = PnP_OutputInterface::micros();

				// next polling is the duration of the OFF state
				PnP_OutputInterface::SetPollingInterval(offDuration);
			}
		} else {
			// On duration didn't pass yet, processing was called before needed
			// Set polling to the time left
			PnP_OutputInterface::SetPollingInterval(onDuration_FadingDuration - timePassed / 1000);
		}
		break;

	case LED_BLINKING_OFF:
		timePassed = PnP_OutputInterface::micros() - effectStart;

		if (timePassed > offDuration * 1000) {
			// Off duration passed, turn the led on and set the data for BLINKING_ON state
			rc = PnP_OutputInterface::SetValue(brightness);

			state = LED_BLINKING_ON;
			effectStart = PnP_OutputInterface::micros();

			// Reduce one repeat count if it's not set to infinite repetition
			if (fadeUpdatePeriod_repeatTimes != INFINITE_REPEAT) {
				fadeUpdatePeriod_repeatTimes--;
			}

			// next polling is the duration of the OFF state
			PnP_OutputInterface::SetPollingInterval(onDuration_FadingDuration);
		} else {
			// On duration didn't pass yet, processing was called before needed
			// Set polling to the time left
			PnP_OutputInterface::SetPollingInterval(offDuration - timePassed / 1000);
		}
		break;

	case LED_FADING_IN:
		timePassed = (PnP_OutputInterface::micros() - effectStart) / 1000;	// in mSec

		// Calculate the brightness after passed time
		if (timePassed >= onDuration_FadingDuration) {
			// effect time passed, set to max brightness
			fadingBrightness = brightness;
		} else {
			fadingBrightness = ((1.0 - (onDuration_FadingDuration - timePassed) / (float)onDuration_FadingDuration) * brightness);
		}

		// Set current brightness level
		rc = PnP_OutputInterface::SetValue(fadingBrightness);

		if (fadingBrightness == brightness) {
			// We've reached max fading level
			state = LED_ON;

			PnP_OutputInterface::SetPollingInterval(EBF_NO_POLLING);
		} else {
			// Need more time for fading
			if (onDuration_FadingDuration - timePassed > fadeUpdatePeriod_repeatTimes) {
				PnP_OutputInterface::SetPollingInterval(fadeUpdatePeriod_repeatTimes);
			} else {
				PnP_OutputInterface::SetPollingInterval(onDuration_FadingDuration - timePassed);
			}
		}
		break;

	case LED_FADING_OUT:
		timePassed = (PnP_OutputInterface::micros() - effectStart) / 1000;	// in mSec

		// Calculate the brightness after passed time
		if (timePassed >= onDuration_FadingDuration) {
			// effect time passed, set to OFF
			fadingBrightness = 0.0;
		} else {
			fadingBrightness = ((onDuration_FadingDuration - timePassed) / (float)onDuration_FadingDuration * brightness);
		}

		// Set current brightness level
		rc = PnP_OutputInterface::SetValue(fadingBrightness);

		if (fadingBrightness == 0) {
			// We've reached min fading level (OFF)
			state = LED_OFF;

			PnP_OutputInterface::SetPollingInterval(EBF_NO_POLLING);
		} else {
			// Need more time for fading
			if (onDuration_FadingDuration - timePassed > fadeUpdatePeriod_repeatTimes) {
				PnP_OutputInterface::SetPollingInterval(fadeUpdatePeriod_repeatTimes);
			} else {
				PnP_OutputInterface::SetPollingInterval(onDuration_FadingDuration - timePassed);
			}
		}
		break;

	default:
		rc = EBF_INVALID_STATE;
		break;
	}

	EBF_REPORT_AND_RETURN(rc);
}
