#include "PnP_ButtonInterface.h"

extern void EBF_EmptyCallback();

PnP_ButtonInterface::PnP_ButtonInterface()
{
	this->onPressCallback = EBF_EmptyCallback;
	this->onLongPressCallback = EBF_EmptyCallback;
	this->onReleaseCallback = EBF_EmptyCallback;

	// Long press will be 3 sec by default
	longPressTime = 3000;

	state = BUTTON_STATE_RELEASED;
}

PnP_ButtonInterface::ButtonEvent PnP_ButtonInterface::ProcessInput(uint8_t value)
{
	// Value "0" is considered as pressed
	// Value "1" is considered as released

	switch(this->state) {
		case BUTTON_STATE_PRESSED:
			if (value == 1) {	// Button was released
				this->state = BUTTON_STATE_RELEASED;
				PnP_InputInterface::SetPollingInterval(EBF_NO_POLLING);

				return BUTTON_EVENT_RELEASE;
			}
		break;

		case BUTTON_STATE_RELEASED:
			if (value == 0) {	// Button was pressed
				// Wait for long-press timeout
				this->state = BUTTON_STATE_WAITING_FOR_LONG_PRESS;

				longPressStart = PnP_InputInterface::millis();

				// Process() function is called every polling interval or on change
				// We will poll every 100mSec till the long press will be detected
				if (longPressTime < PnP_InputInterface::GetPollingInterval()) {
					PnP_InputInterface::SetPollingInterval(100);
				}

				return BUTTON_EVENT_PRESS;
			}
		break;

		case BUTTON_STATE_WAITING_FOR_LONG_PRESS:
			if (value == 1) {	// Button was released
				this->state = BUTTON_STATE_RELEASED;

				PnP_InputInterface::SetPollingInterval(EBF_NO_POLLING);

				return BUTTON_EVENT_RELEASE;
			} else {
				// Wait for long-press timeout
				if((PnP_InputInterface::millis() - longPressStart) >= longPressTime) {
					// It's time...
					this->state = BUTTON_STATE_PRESSED;

					// Polling is no more needed
					PnP_InputInterface::SetPollingInterval(EBF_NO_POLLING);

					return BUTTON_EVENT_LONG_PRESS;
				}
			}
		break;
	}

	// Nothing happened
	return BUTTON_EVENT_NONE;
}

void PnP_ButtonInterface::ExecuteCallback() {
	ExecuteCallbackEx(PnP_InputInterface::GetLastValue());
}

void PnP_ButtonInterface::ExecuteCallbackEx(uint8_t value) {
	ButtonEvent event = ProcessInput(value);

	switch(event) {
		case BUTTON_EVENT_NONE:
			// Nothing should be done
			break;

		case BUTTON_EVENT_PRESS:
			onPressCallback();
			break;

		case BUTTON_EVENT_RELEASE:
			onReleaseCallback();
			break;

		case BUTTON_EVENT_LONG_PRESS:
			onLongPressCallback();
			break;
	}
}

uint8_t PnP_ButtonInterface::Process() {
	ExecuteCallbackEx(PnP_InputInterface::GetLastValue());

	return EBF_OK;
}

uint8_t PnP_ButtonInterface::SetInitialValue(uint8_t value)
{
	if (value == 0) {
		state = BUTTON_STATE_PRESSED;
	} else {
		state = BUTTON_STATE_RELEASED;
	}

	return EBF_OK;
}