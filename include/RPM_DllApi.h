/**
 * @file RPM_DllApi.h
 * @author Hello007
 * @brief Header to be included in RPM DllMain implementations.
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_DLLAPI_H
#define __RPM_DLLAPI_H

#define RPM_DLLAPI_DLLMAIN DllMain
#define RPM_DLLAPI_DLLMAIN_NAME "DllMain"

#endif

#ifdef __cplusplus

#ifndef __RPM_DLLAPI_H_CPPDEF
#define __RPM_DLLAPI_H_CPPDEF

namespace rpm {
	class Module;

	namespace mgr {
		class ModuleManager;
	}
}

namespace rpm {
	/**
	 * @brief Enumeration indicating the reason for calling a DllMain function.
	 * 
	 * This is passed as the 3rd parameter of DllMain.
	 */
	enum DllMainReason {
		/**
		 * @brief DllMain was called on module start.
		 */
		MODULE_LOAD,
		/**
		 * @brief DllMain was called before module unload.
		 */
		MODULE_UNLOAD
	};

	/**
	 * @brief Code to be returned from DllMain to ControlModule.
	 */
	enum DllMainReturnCode {
		/**
		 * @brief All DllMain operations completed succesfully.
		 */
		OK,
		/**
		 * @brief Something within DllMain went wrong. This return code will prevent ModuleManager from further loading this module.
		 */
		FAILURE
	};

	/**
	 * @brief DllMain function prototype.	 
	 */
	typedef DllMainReturnCode (*DllMainFunction)(rpm::mgr::ModuleManager*, Module*, DllMainReason);
}

#include "RPM_Module.h"
#include "RPM_ModuleManager.h"

//Force C linkage
#define RPM_DLLAPI_DLLMAIN_PARAMS rpm::mgr::ModuleManager* mgr, rpm::Module* module, rpm::DllMainReason reason
#define RPM_DLLAPI_DLLMAIN_PROTOTYPE rpm::DllMainReturnCode RPM_DLLAPI_DLLMAIN(RPM_DLLAPI_DLLMAIN_PARAMS)
#define RPM_DLLAPI_DLLMAIN_DEFINE RPM_PUBLIC_EXPORT RPM_DLLAPI_DLLMAIN_PROTOTYPE
#define RPM_DLLAPI_DLLMAIN_DECLARE extern "C" RPM_DLLAPI_DLLMAIN_DEFINE

#endif

#else

#ifndef __RPM_DLLAPI_H_CDEF
#define __RPM_DLLAPI_H_CDEF

typedef enum DllMainReason DllMainReason;
typedef enum DllMainReturnCode DllMainReturnCode;

enum DllMainReason {
	DLL_MODULE_LOAD,
	DLL_MODULE_UNLOAD
};

enum DllMainReturnCode {
	DLL_RETCODE_OK,
	DLL_RETCODE_FAILURE
};

typedef DllMainReturnCode (*DllMainFunction)(void*, void*, DllMainReason);

#define RPM_DLLAPI_DLLMAIN_DEFINE DllMainReturnCode RPM_DLLAPI_DLLMAIN(void* mgr, void* module, DllMainReason reason)
#define RPM_DLLAPI_DLLMAIN_DECLARE RPM_DLLAPI_DLLMAIN_DEFINE

#endif

#endif