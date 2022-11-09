#ifndef __RPM_UTIL_CPP
#define __RPM_UTIL_CPP

#include "RPM_Types.h"
#include "RPM_Control.h"
#include "RPM_Module.h"
#include "RPM_CpuUtil.h"
#include "RPM_Util.h"

namespace rpm {
	u8* Util::GetSymbolAddressAbsolute(Module* m, Symbol* sym) {
		if (!sym) {
			return 0;
		}
		u8* destAddr = 0;

		if (sym->Attr & SymbolAttr::RPM_SYMATTR_IMPORT) { //import symbols can not be resolved
			return destAddr;
		}

		if (sym->Attr & SymbolAttr::RPM_SYMATTR_GLOBAL) { //module-relative
			destAddr = 0;
		}
		else {
			destAddr = m->GetCode();
		}
		destAddr += sym->Addr.RawAddress;

		//RPM_DEBUG_PRINTF("symbol %s addr %x local %d", m->GetString(sym->Name), sym->Addr.Value, sym->Addr.Local);

		if (sym->Type == RPM_SYMTYPE_FUNCTION_THM) {
			destAddr++;
		}

		return destAddr;
	}

	#define RELOC_DEBUG_PRINTF(...) RPM_DEBUG_PRINTF(__VA_ARGS__)
	//#define RELOC_DEBUG_PRINTF(...)

	void Util::DoRelocation(u8* srcAddr, Module* m, Relocation* r) {
		u8* destAddr = 0;
		Symbol* sym = 0;

		sym = m->GetSymbol(r->Source.SymbNo);

		if (!sym) {
			RELOC_DEBUG_PRINTF("Failure to resolve relocation symbol no. %d\n", r->Source.SymbNo);
			return;
		}

		destAddr = GetSymbolAddressAbsolute(m, sym);

		if (destAddr) {
			cpu::CpuRelRequest req;
			req.Source = srcAddr;
			req.Symbol = sym;
			req.Target = destAddr;
			req.Type = r->Target.RelProcType;

			cpu::CpuUtil::ProcessRelRequest(&req);
		}
	}

	RPM_NAMEHASH Util::HashName(const char* name) {
		if (!name) {
			return 0;
		}
		RPM_NAMEHASH hash = 0x811C9DC5; //offset_basis
		char c;
		while (true) {
			c = *name;
			if (!c) {
				break;
			}
			hash = (hash ^ c) * 16777619; //FNV_prime
			name++;
		}
		return hash;
	}

	u32 Util::BinarySearchExportTable(RPM_NAMEHASH key, const RPM_NAMEHASH* array, size_t arraySize) {
		u32 start = 0;
		u32 end = arraySize;
		u32 mid;
		RPM_NAMEHASH val;

		while (start < end) {
			mid = start + ((end - start) >> 1);
			val = array[mid];
			if (val == key) {
				return mid;
			}
			else if (val > key) {
				end = mid;
			}
			else { //val < key
				start = mid + 1;
			}
		}
		return -1;
	}

	rpm::Symbol* Util::BinarySearchImportTable(RPM_NAMEHASH key, rpm::Symbol* array, size_t arraySize) {
		u32 start = 0;
		u32 end = arraySize;
		u32 mid;
		RPM_NAMEHASH val;

		while (start < end) {
			mid = start + ((end - start) >> 1);
			val = array[mid].Addr.ImportHash;
			if (val == key) {
				return &array[mid];
			}
			else if (val > key) {
				end = mid;
			}
			else { //val < key
				start = mid + 1;
			}
		}
		return nullptr;
	}
}

#endif