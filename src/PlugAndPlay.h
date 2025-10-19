#ifndef __PLUGANDPLAY_H__
#define __PLUGANDPLAY_H__

#include <Arduino.h>

#if __has_include("Project_Config.h")
	#include "Project_Config.h"
#endif

#include "../EventBasedFramework/EBF.h"

#include "Core/PnP_StatusLed.h"
#include "Core/PnP_Serial.h"
#include "Core/PnP_UART.h"
#ifndef EBF_REMOVE_SPI_IMPLEMENTATION
#include "Core/PnP_SPI.h"
#endif

#include "Core/PnP_InputInterface.h"
#include "Core/PnP_SwitchInterface.h"
#include "Core/PnP_ButtonInterface.h"
#include "Modules/PnP_Module_STTS22H_TemperatureSensor.h"
#include "Modules/PnP_Module_1Led.h"
#include "Modules/PnP_Module_2Led.h"
#include "Modules/PnP_Module_1Input.h"
#include "Modules/PnP_Module_2Input.h"
#include "Modules/PnP_Module_2ButtonsInput.h"
#include "Modules/PnP_Module_SparkFun_QWIIC_SerLCD.h"
#include "Modules/PnP_Module_Seeed_Monochrome_GROVE_16x2_LCD.h"

#endif
