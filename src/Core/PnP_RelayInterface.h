#ifndef __PNP_RELAY_INTERFACE_H__
#define __PNP_RELAY_INTERFACE_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "PnP_OutputInterface.h"

class PnP_RelayInterface : public PnP_OutputInterface {
	private:
		EBF_DEBUG_MODULE_NAME("PnP_RelayInterface");

	public:
		PnP_RelayInterface() {}

		virtual OutputInterface_Type GetType() { return OutputInterface_Type::RELAY; }

		uint8_t Close();
		uint8_t Open();

		uint8_t SetValue(uint8_t value);
		uint8_t SetValue(float value) { return this->SetValue((uint8_t)value); }	// PnP_OutputInterface override

	protected:
		uint8_t Process() { return EBF_OK; }
};

#endif
