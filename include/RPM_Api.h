/**
 * @file RPM_Api.h
 * @author Hello007
 * @brief Relocatable Program Module library open API include header.
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __RPM_API_H
#define __RPM_API_H

#ifndef RPM_STATIC_LIBRARY
#define RPM_DLLAPI
#endif

#include "RPM_Types.h" //TypeDef

#include "RPM_DllApi.h"

#include "RPM_ModuleInit.h"
#include "RPM_ModuleFixLevel.h"
#include "RPM_Module.h"
#include "RPM_ModuleManager.h"

#endif