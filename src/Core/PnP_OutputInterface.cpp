#include "PnP_OutputInterface.h"

PnP_OutputInterfaceProvider* PnP_OutputInterface::pCurrentProvider = NULL;

PnP_OutputInterface::PnP_OutputInterface()
{
	this->pOutputProvider = NULL;
	this->providerIndex = 0;
}

uint8_t PnP_OutputInterface::AssignInterfaceProvider(PnP_OutputInterfaceProvider* pProvider, uint8_t index)
{
	this->pOutputProvider = pProvider;
	this->providerIndex = index;

	return EBF_OK;
}