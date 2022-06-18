#ifndef __RPM_MODULE_CPP
#define __RPM_MODULE_CPP

#include "RPM_Types.h"
#include "RPM_Module.h"
#include "RPM_Util.h"
#include "RPM_CpuUtil.h"
#include "RPM_Version.h"
#include "RPM_DllApi.h"
#include "RPM_ModuleFixLevel.h"
#include "RPM_ModuleInit.h"
#include <cstring>

namespace rpm {
	Module* Module::InitModule(rpm::init::ModuleAllocation alloc) {
		Module* module = reinterpret_cast<Module*>(alloc);
		module->RelocateControl();
		return module;
	}

	size_t Module::CalcFixedSize(rpm::FixLevel fixLevel) {
		size_t newModuleSize = m_Size;
		if (fixLevel >= rpm::FixLevel::ALL_NONCODE) {
			//newModuleSize = (GetCode() + GetCodeSize()) - reinterpret_cast<u8*>(this);
			newModuleSize = reinterpret_cast<u8*>(&m_Exec->Info) + sizeof(InfoSection) - reinterpret_cast<u8*>(this); //end of the info section
		}
		else if (fixLevel >= rpm::FixLevel::INTERNAL_RELOCATIONS) {
			RelocationSection* relSection = GetRelocations();
			if (relSection) {
				RelocationList* internals = relSection->InternalRelocations;
				if (internals) {
					newModuleSize = reinterpret_cast<u8*>(internals) - reinterpret_cast<u8*>(this);
					RPM_DEBUG_PRINTF("Size after fixing %x\n", newModuleSize);
				}
			}
		}
		if (newModuleSize != m_Size) {
			return newModuleSize;
		}
		return -1;
	}

	const char* Module::GetString(RPM_NAMEOFS offs) {
		if (m_Exec) {
			if (m_Exec->Info) {
				if (m_Exec->Info->Strings) {
					return &m_Exec->Info->Strings->Strings[offs];	
				}
			}
		}
		return nullptr;
	}

	void Module::LinkWithModule(Module* other) {
		this->ImportModule(other);
		other->ImportModule(this);
	}

	void Module::ImportModule(Module* other) {
		RPM_DEBUG_PRINTF("Begin module import.\n");
		SymbolSection* symSect = GetSymbols();
		SymbolSection* otherSymSect = other->GetSymbols();
		if (symSect && otherSymSect && otherSymSect->ExportSymbolHashTable) {
			u32 importSymbolCount = symSect->ImportSymbolCount;
			u32 otherExportSymbolCount = otherSymSect->ExportSymbolCount;
			u8* otherCode = other->GetCode();

			RPM_DEBUG_PRINTF("Linking module, import symbol ct %d other export symbol ct %d\n", importSymbolCount, otherExportSymbolCount);
			
			if (importSymbolCount && otherExportSymbolCount && symSect->FirstExportSymbolIdx != 0xFFFF) {
				Symbol* sym = &symSect->Symbols[symSect->FirstImportSymbolIdx];
				Symbol* extSym;
				u32 importSymbolEnd = symSect->FirstImportSymbolIdx + importSymbolCount;
				for (u32 importSymbolIndex = symSect->FirstImportSymbolIdx; importSymbolIndex < importSymbolEnd; importSymbolIndex++, sym++) {
					if (sym->Attr & SymbolAttr::RPM_SYMATTR_IMPORT) {
						RPM_NAMEHASH hash = sym->Addr.ImportHash;

						RPM_NAMEHASH* exportHashArr = otherSymSect->ExportSymbolHashTable;
						for (u32 j = 0; j < otherExportSymbolCount; j++, exportHashArr++) {
							if (hash == *exportHashArr) {
								//Hashes matched
								extSym = &otherSymSect->Symbols[otherSymSect->FirstExportSymbolIdx + j];
								if (!(extSym->Attr & SymbolAttr::RPM_SYMATTR_IMPORT)) {
									RPM_DEBUG_PRINTF("Linking symbol %s (hash %x).\n", GetString(sym->Name), hash);
									sym->Addr.Internal.Local = 0; //always global offset
									if (extSym->Addr.Internal.Local) {
										sym->Addr.Internal.Value = reinterpret_cast<u32>(otherCode + extSym->Addr.Internal.Value);
									}
									else {
										sym->Addr.Internal.Value = extSym->Addr.Internal.Value;
									}
									sym->Type = extSym->Type;

									sym->Attr &= ~SymbolAttr::RPM_SYMATTR_IMPORT;
									RelocateByImportSymbol(importSymbolIndex);
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	u16 Module::FindSymbolIdx(const char* name) {
		SymbolSection* symbols = GetSymbols();
		if (symbols) {
			Symbol* pSym = symbols->Symbols;
			for (u16 i = 0; i < symbols->SymbolCount; i++, pSym++) {
				const char* other = GetString(pSym->Name);
				if (other) {
					if (name) {
						if (strcmp(other, name) == 0) {
							return i;
						}
					}
				}
				else if (!name) {
					return i;
				}
			}
		}
		return 0xFFFF;
	}

	u16 Module::FindExportSymbolIdx(const char* name) {
		SymbolSection* symbols = GetSymbols();
		if (symbols) {
			if (symbols->ExportSymbolHashTable) {
				RPM_NAMEHASH hash = Util::HashName(name);
				RPM_DEBUG_PRINTF("Looking for export symbol %s by hash %x.\n", name, hash);
				u32 count = symbols->ExportSymbolCount;
				RPM_NAMEHASH* hashes = symbols->ExportSymbolHashTable;
				for (u32 i = 0; i < count; i++, hashes++) {
					if (hash == *hashes) {
						return i + symbols->FirstExportSymbolIdx;
					}
				}
			}
		}
		return 0xFFFF;
	}

	Symbol* Module::FindSymbol(const char* name) {
		u16 index = FindSymbolIdx(name);
		if (index != 0xFFFF) {
			return GetSymbol(index);
		}
		return NULL;
	}

	Symbol* Module::FindExportSymbol(const char* name) {
		u16 index = FindExportSymbolIdx(name);
		if (index != 0xFFFF) {
			return GetSymbol(index);
		}
		return NULL;
	}

	Symbol* Module::GetSymbol(u16 index) {
		SymbolSection* ssec = GetSymbols();
		if (ssec) {
			if (index < ssec->SymbolCount) {
				return &ssec->Symbols[index];
			}
			else {
				RPM_DEBUG_PRINTF("Symbol index %d exceeds maximum of %d!!\n", index, ssec->SymbolCount);
			}
		}
		return NULL;
	}

	u8* Module::GetSymbolAddressAbsolute(Symbol* sym) {
		return Util::GetSymbolAddressAbsolute(this, sym);
	}
	
	u16 Module::GetSymExternModuleCount() {
		SymbolSection* sym = GetSymbols();
		if (sym) {
			if (sym->ExternModules) {
				return sym->ExternModules->Count;
			}
		}
		return 0;
	}

	const char* Module::GetSymExternModuleName(u16 index) {
		SymbolSection* sym = GetSymbols();
		if (sym) {
			if (sym->ExternModules) {
				return GetString(sym->ExternModules->Entries[index]);
			}
		}
		return NULL;
	}

	u16 Module::GetRelExternModuleCount() {
		RelocationSection* rel = GetRelocations();
		if (rel) {
			if (rel->ExternModules) {
				return rel->ExternModules->Count;
			}
		}
		return 0;
	}

	const char* Module::GetRelExternModuleName(u16 index) {
		RelocationSection* rel = GetRelocations();
		if (rel) {
			if (rel->ExternModules) {
				return GetString(rel->ExternModules->Entries[index]);
			}
		}
		return NULL;
	}

	void Module::RelocFilePtr(void* pptr) {
		Util::RelocPtr(pptr, static_cast<void*>(this));
	}

	void Module::RelocFilePtrNonNull(void* pptr) {
		void** test = static_cast<void**>(pptr);
		void* ptr = *test;
		if (ptr != reinterpret_cast<void*>(0xFFFFFFFF)) {
			if (ptr != NULL) {
				RelocFilePtr(pptr);
			}
		}
		else {
			*test = NULL;
		}
	}

	void Module::RelocateControl() {
		if (!GetReserveFlag(RPM_RSVFLAG_CONTROL_RELOCATED)) {
			RelocFilePtr(&m_Exec);
			RelocFilePtrNonNull(&m_Exec->Info);
			RelocFilePtrNonNull(&m_Exec->Info->Code);
			RelocFilePtrNonNull(&m_Exec->Info->Symbols);
			RelocFilePtrNonNull(&m_Exec->Info->Strings);
			RelocFilePtrNonNull(&m_Exec->Info->Relocations);
			RelocFilePtrNonNull(&m_Exec->Info->MetaValueSection);
			if (m_Exec->Info->Symbols) {
				RelocFilePtrNonNull(&m_Exec->Info->Symbols->ExternModules);
				RelocFilePtrNonNull(&m_Exec->Info->Symbols->ExportSymbolHashTable);
			}
			if (m_Exec->Info->Relocations) {
				RelocFilePtrNonNull(&m_Exec->Info->Relocations->InternalRelocations);
				RelocFilePtrNonNull(&m_Exec->Info->Relocations->InternalImportRelocations);
				RelocFilePtrNonNull(&m_Exec->Info->Relocations->ExternalRelocations);
				RelocFilePtrNonNull(&m_Exec->Info->Relocations->ExternModules);
			}
			SetReserveFlag(RPM_RSVFLAG_CONTROL_RELOCATED);
		}
	}

	void Module::RelocateInternal() {
		if (!GetReserveFlag(RPM_RSVFLAG_CODE_RELOCATED_INTERNAL)) {
				RelocationSection* rel = GetRelocations();
				if (rel) {

				RelocationList* internals = rel->InternalRelocations;

				for (int i = 0; i < internals->Count; i++) {
					Relocation* r = &internals->Relocations[i];

					u32 addr = r->Target.Offset;
					Util::CutAlign16(&addr);

					u8* code = GetCode() + addr;

					Util::DoRelocation(code, this, r);
				}

				SetReserveFlag(RPM_RSVFLAG_CODE_RELOCATED_INTERNAL);
			}
		}
	}

	void Module::RelocateByImportSymbol(u32 symIndex) {
		RelocationSection* rel = GetRelocations();
		if (rel) {
			RelocationList* importRels = rel->InternalImportRelocations;

			for (int i = 0; i < importRels->Count; i++) {
				Relocation* r = &importRels->Relocations[i];

				if (r->Source.SymbNo == symIndex) {
					u32 addr = r->Target.Offset;
					Util::CutAlign16(&addr);

					u8* code = GetCode() + addr;

					Util::DoRelocation(code, this, r);
				}
			}
		}
	}

	bool Module::Verify() {
		if (!GetReserveFlag(RPM_RSVFLAG_CONTROL_RELOCATED)) {
			return false;
		}
		if (m_Magic != RPM_MAGIC) {
			return false;
		}
		if (!m_Exec) {
			return false;
		}
		if (m_Exec->Magic != DLLEXEC_MAGIC) {
			return false;
		}
		if (m_Exec->Version != LIBRPM_VERSION) {
			return false;
		}
		if (!m_Exec->Info) {
			return false;
		}
		InfoSection* info = m_Exec->Info;
		if (info->Magic != INFO_MAGIC) {
			return false;
		}
		if (info->Symbols && info->Symbols->Magic != SYM0_MAGIC) {
			return false;
		}
		if (info->Relocations && info->Relocations->Magic != REL0_MAGIC) {
			return false;
		}
		if (info->Strings && info->Strings->Magic != STR0_MAGIC) {
			return false;
		}
		if (info->MetaValueSection && info->MetaValueSection->Magic != META_MAGIC) {
			return false;
		}
		return true;
	}
}

#endif