#include "PnP_RelayInterface.h"

// SetValue acts as an ON/OFF function, value == 0 will perform as OFF (Open), any other value as ON (Close)
uint8_t PnP_RelayInterface::SetValue(uint8_t value)
{
	uint8_t rc;

	if (value == 0) {
		rc = pOutputProvider->SetValue_OIP(providerIndex, 0.0);
	} else {
		rc = pOutputProvider->SetValue_OIP(providerIndex, 100.0);
	}

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_RelayInterface::Close()
{
	uint8_t rc;

	rc = pOutputProvider->SetValue_OIP(providerIndex, 100.0);

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_RelayInterface::Open()
{
	uint8_t rc;

	rc = pOutputProvider->SetValue_OIP(providerIndex, 0.0);

	EBF_REPORT_AND_RETURN(rc);
}

