#ifndef __RPM_CPUUTIL_CPP
#define __RPM_CPUUTIL_CPP

#include "RPM_Types.h"
#include "RPM_Util.h"
#include "RPM_CpuUtil.h"
#include <cstdlib>

namespace rpm {
	namespace cpu {
		void CpuUtil::Reloc_OFFSET(CpuRelRequest* req) {
			Write32(req->Source, reinterpret_cast<u32>(req->Target));
		}

		void CpuUtil::Reloc_THUMB_BL(CpuRelRequest* req) {
			u16 high;
			u16 low;

			u8* prefetchPtr = req->Source + 4;
			ptrdiff_t diff = req->Target - prefetchPtr;
			
			high = THUMB_BL_HI(diff);

			if (!IsAddrThumb(req->Target)) {
				if (diff < 0) {
					diff = (diff + 3) & 0xFFFFFFFC;
				}
				low = THUMB_BLX_LO(diff);
			}
			else {
				low = THUMB_BL_LO(diff);
			}

			StreamWrite16(&req->Source, high);
			StreamWrite16(&req->Source, low);
		}

		void CpuUtil::Reloc_ARM_BL(CpuRelRequest* req) {
			u32 instruction;

			u8* prefetchPtr = req->Source + 8;
			ptrdiff_t diff = req->Target - prefetchPtr;
			
			if (IsAddrThumb(req->Target)) {
				instruction = ARM_BLX(diff);
			}
			else {
				instruction = ARM_BL(diff);
			}

			Write32(req->Source, instruction);
		}

		void CpuUtil::Reloc_THUMB_B(CpuRelRequest* req) {
			u8* prefetchPtr = req->Source + 4;
			ptrdiff_t diff = req->Target - prefetchPtr;
			ptrdiff_t diffAbs = diff < 0 ? -diff : diff;

			if (diffAbs < 2048) {
				Write16(req->Source, THUMB_B(diff));
			}
			else {
				StreamWrite16(&req->Source, THUMB_PUSH_LR);
				Reloc_THUMB_BL(req);
				StreamWrite16(&req->Source, THUMB_POP_PC);
			}
		}

		void CpuUtil::Reloc_ARM_B(CpuRelRequest* req) {
			u8* prefetchPtr = req->Source + 8;
			ptrdiff_t diff = req->Target - prefetchPtr;

			Write32(req->Source, ARM_B(diff));
		}

		void CpuUtil::Reloc_FULL_COPY(CpuRelRequest* req) {
			if (req->Symbol) {
				u16 len = req->Symbol->Size;
				//we can copy 2 bytes at a time since symbols are aligned
				u16* src16 = reinterpret_cast<u16*>(req->Source);
				u16* tgt16 = reinterpret_cast<u16*>(req->Target);
				for (int i = 0; i < len; i += 2) {
					*src16 = *tgt16;
					src16++;
					tgt16++;
				}
			}
		}

		void CpuUtil::Reloc_THUMB_B_SAFESTACK(CpuRelRequest* req) {
			u8* prospectedLDRPtr = req->Source;
			prospectedLDRPtr += 5 * sizeof(u16); //5 instructions

			rpm::Util::StreamAlign32(&prospectedLDRPtr);

			StreamWrite16(&req->Source, THUMB_PUSH_R4);

			ptrdiff_t diff = prospectedLDRPtr - req->Source - 4; //prefetch
			if (diff & 3) {
				diff += 2; //CPU will count from offset with bit 1 unset
			}
			StreamWrite16(&req->Source, THUMB_LDR_PC_REL(4, diff));

			StreamWrite16(&req->Source, THUMB_MOV_HI(12, 4));
			StreamWrite16(&req->Source, THUMB_POP_R4);
			StreamWrite16(&req->Source, THUMB_BX(12));

			req->Source = prospectedLDRPtr;
			StreamWrite32(&req->Source, reinterpret_cast<u32>(req->Target));
		}

		void CpuUtil::Reloc_OFFSET_REL31(CpuRelRequest* req) {
			u32 highBits = *reinterpret_cast<u32*>(req->Source);
			Write32(req->Source, (highBits & 0x80000000) | ((req->Target - req->Source) & 0x7FFFFFFF));
		}

		const RelFunction CpuUtil::REL_FUNCTIONS[] = {
			CpuUtil::Reloc_OFFSET,
			CpuUtil::Reloc_THUMB_BL,
			CpuUtil::Reloc_ARM_BL,
			CpuUtil::Reloc_THUMB_B,
			CpuUtil::Reloc_ARM_B,
			CpuUtil::Reloc_FULL_COPY,
			CpuUtil::Reloc_THUMB_B_SAFESTACK,
			CpuUtil::Reloc_OFFSET_REL31,
		};

		void CpuUtil::ProcessRelRequest(CpuRelRequest* req) {
			if(req->Type < NELEMS(CpuUtil::REL_FUNCTIONS)) {
				//RPM_DEBUG_PRINTF("Call rel func type %d\n", req->Type);
				REL_FUNCTIONS[req->Type](req);
			}
		}
	}
}

#endif