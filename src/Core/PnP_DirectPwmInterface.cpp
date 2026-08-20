#include "PnP_DirectPwmInterface.h"

// Sets PWM timing value
// float percent paramenter: 0=Complete OFF, 50=50% width ON/OFF, 100=Complete ON
uint8_t PnP_DirectPwmInterface::SetValue(float value)
{
	return this->pOutputProvider->SetValue_OIP(providerIndex, value);
}

uint8_t PnP_DirectPwmInterface::GetValue()
{
	return this->pOutputProvider->GetValue_OIP(providerIndex);
}
