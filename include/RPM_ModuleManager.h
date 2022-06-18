/**
 * @file RPM_ModuleManager.h
 * @author Hello007
 * @brief System for loading, managing and linking RPM modules.
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_MODULEMANAGER_H
#define __RPM_MODULEMANAGER_H

#include "Heap/exl_Allocator.h"

#include "RPM_Types.h"
#include "RPM_DllExport.h"
#include "RPM_DllApi.h"
#include "RPM_Module.h"
#include "RPM_Control.h"
#include "RPM_ExternalRelocator.h"
#include "RPM_ModuleInit.h"
#include "RPM_ModuleFixLevel.h"

namespace rpm {
	namespace mgr {
		class ModuleManager {
		private:
			exl::heap::Allocator* m_ModuleHeap;

			rpm::Module* 		m_LastModule;
			ExternalRelocator*	m_ExternRelocator;

		public:
			/**
			 * @brief Creates a ModuleManager bound to a heap space for loaded modules.
			 * 
			 * @param moduleHeap Arbitrary heap location for module storage.
			 */
			RPM_PUBLIC ModuleManager(exl::heap::Allocator* moduleHeap);

			/**
			 * @brief Binds an interface for processing external relocations.
			 * 
			 * @param relocator An ExternalRelocator, or null to disable external relocation.
			 */
			RPM_PUBLIC void BindExternalRelocator(ExternalRelocator* relocator);

			/**
			 * @brief Allocates memory on the module heap space intended for executable storage.
			 * 
			 * @param size Size of the executable to be loaded.
			 * @return Pointer to the allocation area as a module prototype.
			 */
			RPM_PUBLIC rpm::init::ModuleAllocation AllocModule(size_t size);

			/**
			 * @brief Frees memory occupied by a module. Note that this will by itself not unload the module.
			 * 
			 * @param module The module to discard.
			 */
			RPM_PUBLIC void FreeModule(rpm::Module* module);

			/**
			 * @brief Allocates memory on the module heap space intended for a module's fixed work area.
			 * This should not be used more than once per module to prevent heap fragmentation.
			 * 
			 * @param size Size of the work area.
			 * @return Pointer to the allocated work memory.
			 */
			RPM_PUBLIC void* AllocModuleWorkMemory(size_t size);

			/**
			 * @brief Frees work memory allocated on the module heap.
			 * 
			 * @param mem Pointer to the memory block to discard.
			 */
			RPM_PUBLIC void FreeModuleWorkMemory(void* mem);

			/**
			 * @brief Loads a module to the ModuleManager's domain. This will store it in the module chain and relocate its control sections.
			 * 
			 * @param data The module prototype.
			 * @return Module constructed and loaded from the prototype.
			 */
			RPM_PUBLIC rpm::Module* LoadModule(rpm::init::ModuleAllocation data);

			/**
			 * @brief 'Fixes' a module for optimized memory consumption. This can trim several parts of its memory depending on the FixLevel.
			 * A module may be fixed more than once, but if so, the FixLevel must be incremental.
			 * 
			 * @param module The module to fix.
			 * @param fixLevel New FixLevel to use.
			 */
			RPM_PUBLIC void FixModule(rpm::Module* module, rpm::FixLevel fixLevel);

			/**
			 * @brief Terminates a module, removes it from the module chain, and frees it.
			 * 
			 * DllMain(MODULE_UNLOAD) will be invoked before unloading if present.
			 * 
			 * @param module The module to unload.
			 */
			RPM_PUBLIC void UnloadModule(rpm::Module* module);

			/**
			 * @brief Starts up a loaded module, optionally performing a module fix before any initializers are executed.
			 * This will link the module and perform internal relocations, followed by fixing the module and calling DllMain(MODULE_LOAD) if present.
			 * 
			 * @param module The module to start.
			 * @param fixLevel Level of fixing to perform between relocation and calling DllMain. 
			 */
			RPM_PUBLIC void StartModule(rpm::Module* module, rpm::FixLevel fixLevel);

			/**
			 * @brief Calls the DllMain function of a module, if it is present.
			 * 
			 * @param module The module to control.
			 * @param reason Reason parameter for the DllMain function.
			 * @return Exit code returned by DllMain.
			 */
			RPM_PUBLIC rpm::DllMainReturnCode ControlModule(rpm::Module* module, rpm::DllMainReason reason);

			/**
			 * @brief Links a module against all of the current module chain.
			 * 
			 * @param module The module to link.
			 */
			void LinkModule(rpm::Module* module);

			/**
			 * @brief Uses the currently bound external relocator to perform all external relocations that target a given module.
			 * 
			 * @param module The module hosting the relocations.
			 * @param externModule Implementation-defined tag of the external module.
			 */
			RPM_PUBLIC void LinkModuleExtern(rpm::Module* module, const char* externModule);
		};
	}
}

#endif