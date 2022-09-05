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

		if (sym->Addr.Internal.Local) { //module-relative
			destAddr = m->GetCode();
		}
		else {
			destAddr = 0;
		}
		destAddr += sym->Addr.Internal.Value;

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
}

#endif