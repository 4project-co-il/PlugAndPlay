#include "PnP_IntLineInterface.h"

uint8_t PnP_IntLineInterface::SetIntLine(PnP_PlugAndPlayI2C* pPnPI2C, uint8_t line, uint8_t value)
{
	uint8_t rc;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();

	// Line can be only 0 or 1 (the interrupt line number)
	if (line > 1) {
		EBF_REPORT_AND_RETURN(EBF_INDEX_OUT_OF_BOUNDS);
	}

	rc = pHub->SetIntLine(pPnPI2C->GetPortNumber(), line, value & 0x03);

	EBF_REPORT_AND_RETURN(rc);
}

uint8_t PnP_IntLineInterface::GetIntLine(PnP_PlugAndPlayI2C* pPnPI2C, uint8_t line, uint8_t &value)
{
	uint8_t rc;
	PnP_PlugAndPlayHub *pHub = pPnPI2C->GetHub();

	// Line can be only 0 or 1 (the interrupt line number)
	if (line > 1) {
		EBF_REPORT_AND_RETURN(EBF_INDEX_OUT_OF_BOUNDS);
	}

	rc = pHub->GetIntLine(pPnPI2C->GetPortNumber(), line, value);

	EBF_REPORT_AND_RETURN(rc);
}
