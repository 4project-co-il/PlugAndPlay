#include "PnP_Module_2ButtonsInput.h"

extern void EBF_EmptyCallback();

PnP_Module_2ButtonsInput::PnP_Module_2ButtonsInput() : PnP_Module_2Inputs()
{
	this->type = HAL_Type::PnP_DEVICE;
	this->id = PnP_DeviceId::PNP_ID_2BUTTONS_INPUT;
}
