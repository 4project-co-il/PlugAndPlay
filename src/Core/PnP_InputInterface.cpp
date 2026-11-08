#include "PnP_InputInterface.h"

PnP_InputInterfaceProvider* PnP_InputInterface::pCurrentProvider = NULL;

PnP_InputInterface::PnP_InputInterface()
{
	this->pInputProvider = NULL;
	this->providerIndex = 0;
	this->pollingInterval = EBF_NO_POLLING;
}

uint8_t PnP_InputInterface::AssignInterfaceProvider(PnP_InputInterfaceProvider* pProvider, uint8_t index)
{
	if (this->pInputProvider != NULL) {
		return EBF_INVALID_STATE;
	}

	this->pInputProvider = pProvider;
	this->providerIndex = index;

	return EBF_OK;
}

void PnP_InputInterface::SetPollingInterval(uint32_t ms)
{
	this->pollingInterval = ms;

	pInputProvider->SetPollingInterval_IIP(ms);
}