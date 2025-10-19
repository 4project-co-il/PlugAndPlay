#ifndef EBF_REMOVE_SPI_IMPLEMENTATION

#ifndef _PNP_SPI_H__
#define _PNP_SPI_H__

#include <Arduino.h>
#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../../../EventBasedFramework/src/Core/EBF_Global.h"
#include "../../../EventBasedFramework/src/Core/EBF_HalInstance.h"
#include "../../../EventBasedFramework/src/Core/EBF_Core.h"
#include "../../../EventBasedFramework/src/Core/EBF_Logic.h"
#include "../../../EventBasedFramework/src/Core/EBF_SPI.h"

// PnP_SPI class wraps the EBF_SPI implementation, using the defaukt SPI interface
class PnP_SPI : public EBF_SPI {
	public:
		PnP_SPI();

		uint8_t Init();
};

#endif

#endif	// EBF_REMOVE_SPI_IMPLEMENTATION
