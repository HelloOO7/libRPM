#ifndef __RPM_MODULEMANAGER_CPP
#define __RPM_MODULEMANAGER_CPP

#include "Heap/exl_Allocator.h"

#include "RPM_Types.h"
#include "RPM_DllApi.h"
#include "RPM_Module.h"
#include "RPM_ModuleManager.h"
#include "RPM_ModuleInit.h"
#include "RPM_Util.h"

namespace rpm {
	namespace mgr {
		ModuleManager::ModuleManager(exl::heap::Allocator* moduleHeap) {
			m_LastModule = NULL;
			m_ExternRelocator = NULL;
			m_ModuleHeap = moduleHeap;
		}

		rpm::init::ModuleAllocation ModuleManager::AllocModule(size_t size) {
			return m_ModuleHeap->Alloc(size);
		}

		void ModuleManager::FreeModule(rpm::Module* module) {
			m_ModuleHeap->Free(module);
		}

		void* ModuleManager::AllocModuleWorkMemory(size_t size) {
			return m_ModuleHeap->Alloc(size);
		}

		void ModuleManager::FreeModuleWorkMemory(void* mem) {
			m_ModuleHeap->Free(mem);
		}

		void ModuleManager::BindExternalRelocator(ExternalRelocator* relocator) {
			m_ExternRelocator = relocator;
		}

		rpm::Module* ModuleManager::LoadModule(rpm::init::ModuleAllocation data) {
			rpm::Module* module = rpm::Module::InitModule(data);

			if (m_LastModule) {
				m_LastModule->SetNextModule(module);
				module->SetPrevModule(m_LastModule);
				m_LastModule = module;
			}
			else {
				m_LastModule = module;
			}

			if (!module->Verify()) {
				RPM_DEBUG_PRINTF("Module verification failed!!");
			}

			return module;
		}

		void ModuleManager::UnloadModule(rpm::Module* module) {
			ControlModule(module, rpm::DllMainReason::MODULE_UNLOAD);
			if (module->GetPrevModule()) {
				module->GetPrevModule()->SetNextModule(module->GetNextModule());
			}
			if (module->GetNextModule()) {
				module->GetNextModule()->SetPrevModule(module->GetPrevModule());
			}
			if (module == m_LastModule) {
				//Last module shall only have a PrevModule, not NextModule
				//If module->PrevModule is NULL, this is the last module being unloaded
				m_LastModule = module->GetPrevModule();
			}
			FreeModule(module);
		}

		void ModuleManager::StartModule(rpm::Module* module, rpm::FixLevel fixLevel) {
			rpm::Util::ForbidUnused(0x42694269);
			RPM_DEBUG_PRINTF("Starting module...\n");
			RPM_DEBUG_PRINTF("Linking...\n");
			LinkModule(module);
			RPM_DEBUG_PRINTF("Processing internal relocations...\n");
			module->RelocateInternal();
			RPM_DEBUG_PRINTF("Fixing %d.\n", fixLevel);
			FixModule(module, fixLevel);
			//TODO static initializers
			ControlModule(module, rpm::DllMainReason::MODULE_LOAD); //todo: failure ?
		}

		void ModuleManager::FixModule(rpm::Module* module, rpm::FixLevel fixLevel) {
			size_t fixedSize = module->CalcFixedSize(fixLevel);
			if (fixedSize != -1) {
				m_ModuleHeap->Realloc(module, fixedSize);

				switch (fixLevel) {
					case rpm::FixLevel::INTERNAL_RELOCATIONS:
					{
						rpm::Module::RelocationSection* rels = module->GetRelocations();
						rels->InternalRelocations = nullptr;
						break;
					}
					case rpm::FixLevel::ALL_NONCODE:
						module->DisableControl();
						break;
				}

				module->UpdateModuleSizeAfterFixing(fixedSize);
			}
		}

		rpm::DllMainReturnCode ModuleManager::ControlModule(rpm::Module* module, rpm::DllMainReason reason) {
			RPM_DEBUG_PRINTF("ControlModule begin\n");
			Symbol* sym = module->FindExportSymbol(RPM_DLLAPI_DLLMAIN_NAME);
			if (sym) {
				DllMainFunction func = reinterpret_cast<DllMainFunction>(module->GetSymbolAddressAbsolute(sym));

				#ifndef _WIN32 //do not execute ARM RPM code on _WIN32
				return func(this, module, reason);
				#else
				RPM_DEBUG_PRINTF("Run DllMain for reason %d.\n", reason);
				return DllMainReturnCode::OK;
				#endif
			}

			return DllMainReturnCode::FAILURE;
		}

		void ModuleManager::LinkModule(rpm::Module* module) {
			rpm::Module* other = m_LastModule;
			while (other) {
				if (other != module) {
					module->LinkWithModule(other);
				}
				other = other->GetPrevModule();
			}
		}

		void ModuleManager::LinkModuleExtern(rpm::Module* module, const char* externModule) {
			if (m_ExternRelocator) {
				rpm::Module::RelocationSection* rel = module->GetRelocations();
				if (rel) {
					rpm::RelocationList* externals = rel->ExternalRelocations;
					
					if (externModule) {
						if (rel->ExternModules) {
							u32 extModIndex = -1;

							{
								rpm::ModuleNameList* list = rel->ExternModules;
								u32 extModCount = list->Count;
								for (u32 i = 0; i < extModCount; i++) {
									RPM_NAMEOFS nameofs = list->Entries[i];
									if (!strcmp(externModule, module->GetString(nameofs))) {
										extModIndex = i;
										break;
									}
								}
							}

							if (extModIndex != -1) {
								for (int i = 0; i < externals->Count; i++) {
									Relocation* r = &externals->Relocations[i];

									if (r->Target.ExternModuleIndex == extModIndex) {
										m_ExternRelocator->ProcessRelocation(module, r);
									}
								}
							}
						}
					}
					else {
						for (int i = 0; i < externals->Count; i++) {
							Relocation* r = &externals->Relocations[i];

							m_ExternRelocator->ProcessRelocation(module, r);
						}
					}
				}
			}
		}
	}
}

#endif