#ifndef __RPM_MODULEMANAGER_CPP
#define __RPM_MODULEMANAGER_CPP

#include "Heap/exl_Allocator.h"
#include "Util/exl_StrEq.h"

#include "RPM_Types.h"
#include "RPM_DllApi.h"
#include "RPM_Module.h"
#include "RPM_ModuleManager.h"
#include "RPM_ModuleInit.h"
#include "RPM_Util.h"

namespace rpm {
	namespace mgr {
		ModuleManager::ModuleManager(exl::heap::Allocator* moduleHeap) {
			m_LastModule = nullptr;
			m_ExternRelocator = nullptr;
			m_ListenerHead = nullptr;
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

		void ModuleManager::BindModuleListener(ModuleListener* listener) {
			listener->m_Next = m_ListenerHead;
			m_ListenerHead = listener;
		}

		void ModuleManager::CallModuleListeners(rpm::Module* module, ModuleEvent event) {
			ModuleListener* l = m_ListenerHead;
			while (l) {
				l->OnEvent(this, module, event);
				l = l->m_Next;
			}
		}

		void CallFuncArray(rpm::Module* mod, rpm::FuncArrayList* funcArray) {
			if (funcArray) {
				for (u32 i = 0; i < funcArray->Count; i++) {
					rpm::Symbol* sym = mod->GetSymbol(funcArray->SymbolIndices[i]);
					if (sym) {
						VoidFn* funcptr = reinterpret_cast<VoidFn*>(mod->GetSymbolAddressAbsolute(sym));
						u32 fnCount = (sym->Size) >> 2;
						for (u32 fnIndex = 0; fnIndex < fnCount; fnIndex++) {
							(*funcptr)();
							funcptr++;
						}
					}
				}
			}
		}

		rpm::Module* ModuleManager::LoadModule(rpm::init::ModuleAllocation data) {
			RPM_ASSERT(data);
			//Reallocate for BSS expansion. If the parent framework is smart, the allocation is already big enough and nothing is changed.
			data = exl::heap::Allocator::ReallocStatic(data, reinterpret_cast<rpm::Module*>(data)->GetModuleSize());
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
				m_ModuleHeap->Free(data);
				return nullptr;
			}
			CallModuleListeners(module, LOADED);

			return module;
		}

		void ModuleManager::UnloadModule(rpm::Module* module) {
			RPM_ASSERT(module);
			bool started = module->GetReserveFlag(rpm::Module::ReserveFlag::RPM_RSVFLAG_MODULE_STARTED);
			if (started) {
				ControlModule(module, rpm::DllMainReason::MODULE_UNLOAD);
			}
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
			if (started) {
				CallFuncArray(module, module->m_Exec->Info->StaticDestructors);
				module->ClearReserveFlag(rpm::Module::ReserveFlag::RPM_RSVFLAG_MODULE_STARTED);
			}
			UnlinkModule(module);
			CallModuleListeners(module, UNLOADED);
			FreeModule(module);
		}

		void ModuleManager::StartModule(rpm::Module* module, rpm::FixLevel fixLevel) {
			RPM_ASSERT(module);
			RPM_DEBUG_PRINTF("Starting module...\n");
			RPM_DEBUG_PRINTF("Linking...\n");
			LinkModule(module);
			RPM_DEBUG_PRINTF("Processing internal relocations...\n");
			module->RelocateInternal();
			CallFuncArray(module, module->m_Exec->Info->StaticInitializers);
			RPM_DEBUG_PRINTF("Fixing %d.\n", fixLevel);
			FixModule(module, fixLevel);
			CallModuleListeners(module, READY);
			CallModuleListeners(module, EXEC_UPDATED);
			//TODO static initializers
			ControlModule(module, rpm::DllMainReason::MODULE_LOAD); //todo: failure ?
			CallModuleListeners(module, STARTED);
			module->SetReserveFlag(rpm::Module::ReserveFlag::RPM_RSVFLAG_MODULE_STARTED);
			RPM_DEBUG_PRINTF("Module started\n");
		}

		void* ModuleManager::GetProcAddress(rpm::Module* module, const char* name) {
			if (module) {
				return module->GetProcAddress(name);
			}
			return nullptr;
		}

		void ModuleManager::FixModule(rpm::Module* module, rpm::FixLevel fixLevel) {
			size_t fixedSize = module->CalcFixedSize(fixLevel);
			if (fixedSize != -1) {
				module = static_cast<rpm::Module*>(m_ModuleHeap->Realloc(module, fixedSize)); 
				//The realloc should NEVER return a different pointer as the size is shrinking, but just for sanity...
				RPM_ASSERT(module);

				switch (fixLevel) {
					case rpm::FixLevel::INTERNAL_RELOCATIONS:
					{
						rpm::Module::RelocationSection* rels = module->GetRelocations();
						if (rels) {
							rels->InternalRelocations = nullptr;
						}
						break;
					}
					case rpm::FixLevel::ALL_NONCODE:
						module->DisableControl();
						break;
				}

				module->UpdateModuleSizeAfterFixing(fixedSize);
				CallModuleListeners(module, FIXED);
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
			module->AllowLinking();
			rpm::Module* other = m_LastModule;
			while (other) {
				if (other != module) {
					if (module->LinkWithModule(other)) {
						CallModuleListeners(other, EXEC_UPDATED);
					}
				}
				other = other->GetPrevModule();
			}
			CallModuleListeners(module, EXEC_UPDATED);
		}

		void ModuleManager::UnlinkModule(rpm::Module* module) {
			rpm::Module* other = m_LastModule;
			while (other) {
				if (other != module) {
					module->UnlinkFromModule(other);
					CallModuleListeners(other, EXEC_UPDATED);
				}
				other = other->GetPrevModule();
			}
			CallModuleListeners(module, EXEC_UPDATED);
		}

		void ModuleManager::LinkModuleExtern(rpm::Module* module, const char* externModule) {
			if (m_ExternRelocator) {
				rpm::Module::RelocationSection* rel = module->GetRelocations();
				if (rel) {
					rpm::RelocationList* externals = rel->ExternalRelocations;
					
					if (externals) {
						if (externModule) {
							if (rel->ExternModules) {
								u32 extModIndex = -1;

								{
									rpm::ModuleNameList* list = rel->ExternModules;
									u32 extModCount = list->Count;
									for (u32 i = 0; i < extModCount; i++) {
										RPM_NAMEOFS nameofs = list->Entries[i];
										if (strequal(externModule, module->GetString(nameofs))) {
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
}

#endif