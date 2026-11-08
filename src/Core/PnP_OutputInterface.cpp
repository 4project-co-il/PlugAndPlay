#include "PnP_OutputInterface.h"

PnP_OutputInterface::PnP_OutputInterface()
{
	this->pOutputProvider = NULL;
	this->providerIndex = 0;
	this->pollingInterval = EBF_NO_POLLING;
}

uint8_t PnP_OutputInterface::AssignInterfaceProvider(PnP_OutputInterfaceProvider* pProvider, uint8_t index)
{
	if (this->pOutputProvider != NULL) {
		return EBF_INVALID_STATE;
	}

	this->pOutputProvider = pProvider;
	this->providerIndex = index;

	return EBF_OK;
}

void PnP_OutputInterface::SetPollingInterval(uint32_t ms)
{
	this->pollingInterval = ms;

	pOutputProvider->SetPollingInterval_OIP(ms);
}