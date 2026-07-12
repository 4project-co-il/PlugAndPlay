#ifndef __PNP_MODULE_2BUTTONS_INPUT_H__
#define __PNP_MODULE_2BUTTONS_INPUT_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "PnP_Module_2Input.h"

class PnP_Module_2ButtonsInput : public PnP_Module_2Input {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_Module_2ButtonsInput");

	public:
		PnP_Module_2ButtonsInput();

		static const uint8_t numberOfButtons = PnP_Module_2Input::numberOfInputs;
};

#endif
