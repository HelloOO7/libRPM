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
#include "Util/exl_StrEq.h"
#include <cstring>

namespace rpm {
	Module* Module::InitModule(rpm::init::ModuleAllocation alloc) {
		RPM_ASSERT(alloc);
		Module* module = reinterpret_cast<Module*>(alloc);
		module->Expand();
		module->RelocateControl();
		module->Prepare();
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

	void Module::AllowLinking() {
		SetReserveFlag(RPM_RSVFLAG_MODULE_LINK_READY);
	}

	bool Module::LinkWithModule(Module* other) {
		RPM_ASSERT(other);
		this->ImportModule(other);
		return other->ImportModule(this) != 0;
	}

	void Module::UnlinkFromModule(Module* other) {
		this->UnimportModule(other);
		other->UnimportModule(this);
	}

	u32 Module::ImportModule(Module* other) {
		if (GetReserveFlag(RPM_RSVFLAG_ALL_IMPORTED) || !GetReserveFlag(RPM_RSVFLAG_MODULE_LINK_READY)) {
			return 0;
		}
		RPM_DEBUG_PRINTF("Begin module import.\n");
		SymbolSection* symSect = GetSymbols();
		SymbolSection* otherSymSect = other->GetSymbols();
		u32 totalImportedCount = 0;
		if (symSect && otherSymSect && otherSymSect->ExportSymbolHashTable) {
			u32 firstImportSymbolIdx = symSect->FirstImportSymbolIdx;
			u32 importSymbolCount = symSect->ImportSymbolCount;
			u32 otherExportSymbolCount = otherSymSect->ExportSymbolCount;
			u8* otherCode = other->GetCode();
			RPM_NAMEHASH* exportHashArr = otherSymSect->ExportSymbolHashTable;

			RPM_DEBUG_PRINTF("Linking module, import symbol ct %d other export symbol ct %d first import symbol index %d\n", importSymbolCount, otherExportSymbolCount, firstImportSymbolIdx);
			
			if (importSymbolCount && otherExportSymbolCount && firstImportSymbolIdx != 0xFFFF) {
				Symbol* sym = &symSect->Symbols[firstImportSymbolIdx];
				Symbol* extSym;
				bool existAnyImportSymbol = false;
				u32 importSymbolEnd = firstImportSymbolIdx + importSymbolCount;
				for (u32 importSymbolIndex = firstImportSymbolIdx; importSymbolIndex < importSymbolEnd; importSymbolIndex++, sym++) {
					if (sym->Attr & SymbolAttr::RPM_SYMATTR_IMPORT) {
						RPM_NAMEHASH hash = sym->Addr.ImportHash;
						u32 index = Util::BinarySearchExportTable(hash, exportHashArr, otherExportSymbolCount);
						if (index != -1) {
							//Hashes matched
							extSym = &otherSymSect->Symbols[otherSymSect->FirstExportSymbolIdx + index];
							if (!(extSym->Attr & SymbolAttr::RPM_SYMATTR_IMPORT)) {
								RPM_DEBUG_PRINTF("Linking symbol %s (hash %x).\n", GetString(sym->Name), hash);
								sym->Attr |= RPM_SYMATTR_GLOBAL; //always global offset
								if (!(extSym->Attr & RPM_SYMATTR_GLOBAL)) {
									sym->Addr.RawAddress = reinterpret_cast<u32>(otherCode + extSym->Addr.RawAddress);
								}
								else {
									sym->Addr.RawAddress = extSym->Addr.RawAddress;
								}
								sym->Type = extSym->Type;

								sym->Attr &= ~SymbolAttr::RPM_SYMATTR_IMPORT;
								RelocateByImportSymbol(importSymbolIndex);
								totalImportedCount++;
							}
						}
						else {
							existAnyImportSymbol = true; //there are still symbols yet to be imported
						}
					}
				}
				if (!existAnyImportSymbol) {
					SetReserveFlag(RPM_RSVFLAG_ALL_IMPORTED);
				}
			}
		}
		RPM_DEBUG_PRINTF("Imported %d symbols.\n", totalImportedCount);
		return totalImportedCount;
	}

	void Module::UnimportModule(Module* other) {
		SymbolSection* symSect = GetSymbols();
		SymbolSection* otherSymSect = other->GetSymbols();

		if (symSect && otherSymSect && otherSymSect->ExportSymbolHashTable) {
			u32 firstImportSymbolIdx = symSect->FirstImportSymbolIdx;
			u32 otherExportSymbolCount = otherSymSect->ExportSymbolCount;
			
			if (symSect->ImportSymbolCount && otherExportSymbolCount && firstImportSymbolIdx != 0xFFFF) {
				rpm::Symbol* symArray = &symSect->Symbols[firstImportSymbolIdx];
				RPM_NAMEHASH* exportHashArr = otherSymSect->ExportSymbolHashTable;
				u32 importSymCount = symSect->ImportSymbolCount;
				bool anyUnimported = false;
				for (u32 i = 0; i < otherExportSymbolCount; i++) {
					RPM_NAMEHASH exportedHash = exportHashArr[i];
					rpm::Symbol* imSym = Util::BinarySearchImportTable(exportedHash, symArray, importSymCount);
					if (imSym != nullptr) {
						RPM_DEBUG_PRINTF("Unlinked symbol 0x%x.\n", imSym->Addr.ImportHash);
						imSym->Attr |= SymbolAttr::RPM_SYMATTR_IMPORT; //flag as needs-import
						anyUnimported = true;
					}
				}
				if (anyUnimported) {
					ClearReserveFlag(RPM_RSVFLAG_ALL_IMPORTED);
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
						if (strequal(other, name)) {
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
				u32 index = Util::BinarySearchExportTable(hash, symbols->ExportSymbolHashTable, symbols->ExportSymbolCount);
				if (index != -1) {
					return index + symbols->FirstExportSymbolIdx;
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

	void Module::RelocHeaderPtr(void* pptr) {
		Util::RelocPtr(pptr, static_cast<void*>(m_Exec));
	}

	void Module::RelocHeaderPtrNonNull(void* pptr) {
		void** test = static_cast<void**>(pptr);
		void* ptr = *test;
		if (ptr != reinterpret_cast<void*>(0xFFFFFFFF)) {
			if (ptr != NULL) {
				RelocHeaderPtr(pptr);
			}
		}
		else {
			*test = NULL;
		}
	}

	void Module::Expand() {
		Util::RelocPtr(&m_Exec, this);
		u32 bssSize = m_Exec->BSSSize;
		if (bssSize > 0) {
			DllExec* newHeaderPos = reinterpret_cast<DllExec*>(reinterpret_cast<char*>(m_Exec) + bssSize);
			void* bssStart = m_Exec;
			memmove(newHeaderPos, m_Exec, m_Exec->HeaderSectionSize);
			memset(bssStart, 0, bssSize); //Fill BSS
			m_Exec = newHeaderPos;
		}
	}

	void Module::RelocateControl() {
		if (!GetReserveFlag(RPM_RSVFLAG_CONTROL_RELOCATED)) {
			RelocHeaderPtrNonNull(&m_Exec->Info);
			Util::RelocPtr(&m_Exec->Info->Code, this); //Relative to file, not header
			RelocHeaderPtrNonNull(&m_Exec->Info->Symbols);
			RelocHeaderPtrNonNull(&m_Exec->Info->Strings);
			RelocHeaderPtrNonNull(&m_Exec->Info->Relocations);
			RelocHeaderPtrNonNull(&m_Exec->Info->MetaValueSection);
			RelocHeaderPtrNonNull(&m_Exec->Info->StaticInitializers);
			RelocHeaderPtrNonNull(&m_Exec->Info->StaticDestructors);
			if (m_Exec->Info->Symbols) {
				RelocHeaderPtrNonNull(&m_Exec->Info->Symbols->ExternModules);
				RelocHeaderPtrNonNull(&m_Exec->Info->Symbols->ExportSymbolHashTable);
			}
			if (m_Exec->Info->Relocations) {
				RelocHeaderPtrNonNull(&m_Exec->Info->Relocations->InternalRelocations);
				RelocHeaderPtrNonNull(&m_Exec->Info->Relocations->InternalImportRelocations);
				RelocHeaderPtrNonNull(&m_Exec->Info->Relocations->ExternalRelocations);
				RelocHeaderPtrNonNull(&m_Exec->Info->Relocations->ExternModules);
			}
			SetReserveFlag(RPM_RSVFLAG_CONTROL_RELOCATED);
		}
	}

	void Module::Prepare() {
		rpm::Module::SymbolSection* symSect = GetSymbols();
		if (symSect == nullptr || symSect->ImportSymbolCount == 0) {
			SetReserveFlag(RPM_RSVFLAG_ALL_IMPORTED);
		}
	}

	void Module::RelocateInternal() {
		if (!GetReserveFlag(RPM_RSVFLAG_CODE_RELOCATED_INTERNAL)) {
			RelocationSection* rel = GetRelocations();
			if (rel) {
				RelocationList* internals = rel->InternalRelocations;

				if (internals) {
					for (int i = 0; i < internals->Count; i++) {
						Relocation* r = &internals->Relocations[i];

						u32 addr = r->Target.Offset;
						Util::CutAlign16(&addr);

						u8* code = GetCode() + addr;

						Util::DoRelocation(code, this, r);
					}
				}

				SetReserveFlag(RPM_RSVFLAG_CODE_RELOCATED_INTERNAL);
			}
		}
	}

	void Module::RelocateByImportSymbol(u32 symIndex) {
		RelocationSection* rel = GetRelocations();
		if (rel) {
			RelocationList* importRels = rel->InternalImportRelocations;

			if (importRels) {
				for (int i = 0; i < importRels->Count; i++) {
					Relocation* r = &importRels->Relocations[i];

					if (r->Source.SymbNo == symIndex) {
						u32 addr = r->Target.Offset;
						Util::CutAlign16(&addr);

						u8* code = GetCode() + addr;
						RPM_DEBUG_PRINTF("Relocating by import symbol @ %p (rel. %p) -> %p\n", code, addr);

						Util::DoRelocation(code, this, r);
					}
				}
			}
		}
	}

	bool Module::Verify() {
		if (!GetReserveFlag(RPM_RSVFLAG_CONTROL_RELOCATED)) {
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