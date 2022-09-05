/**
 * @file RPM_Util.h
 * @author Hello007
 * @brief Common RPM utility functions.
 * @version 0.1
 * @date 2022-01-17
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_UTIL_H
#define __RPM_UTIL_H

#include "RPM_Types.h"

#define MAGIC(a,b,c,d) static_cast<u32>((static_cast<u8>(a) <<  0) | (static_cast<u8>(b) << 8) | (static_cast<u8>(c) << 16) | (static_cast<u8>(d) << 24))

#ifdef DEBUG
#include "exl_DebugPrint.h"
#include "exl_Assert.h"
#define RPM_DEBUG_PRINTF exlPrintf
#define RPM_ASSERT(expr) EXL_ASSERT(expr)
#else
#define RPM_DEBUG_PRINTF(...)
#define RPM_ASSERT(expr)
#endif

#include "RPM_Control.h"
#include "RPM_Module.h"
#include "RPM_CpuUtil.h"

namespace rpm {
	class Util {
	public:
		/**
		 * @brief Relocates a pointer with a base address.
		 * 
		 * @param pptr Memory location of the pointer.
		 * @param base Address that the pointer is currently relative to.
		 */
		static INLINE void RelocPtr(void* pptr, void* base) {
			void** vpp = (void**)pptr;
			*vpp = (void*)((size_t)(*vpp) + (size_t)(base));
		}

		/**
		 * @brief Aligns a value down to 2 bytes.
		 * 
		 * @param value Pointer to the value to be aligned.
		 */
		static INLINE void CutAlign16(u32* value) {
			*value &= 0xFFFFFFFE;
		}

		/**
		 * @brief Aligns a value down to 4 bytes.
		 * 
		 * @param value Pointer to the value to be aligned.
		 */
		static INLINE void CutAlign32(u32* value) {
			*value &= 0xFFFFFFFC;
		}

		/**
		 * @brief Aligns a byte stream forward to a 4-byte boundary.
		 * 
		 * @param pStream Pointer to the byte stream.
		 */
		static INLINE void StreamAlign32(u8** pStream) {
			size_t value = reinterpret_cast<size_t>(*pStream);
			if (value & 3) {
				value += 4 - (value & 3);
			}
			*pStream = reinterpret_cast<u8*>(value);
		}

		/**
		 * @brief Dummy function to insert an integer constant to libRPM code for low-level debugging.
		 * 
		 * The value is assigned to a dummy volatile variable to prevent being optimized out.
		 * 
		 * @param value An arbitrary magic value.
		 */
		static void ForbidUnused(u32 value) {
			volatile u32 dummy = value;
		}

		/**
		 * @brief Resolves the physical memory address of a loaded symbol.
		 * 
		 * @param m Parent module of the symbol to process.
		 * @param sym The symbol to process.
		 * @return u8* The memory address of the symbol as a char*.
		 */
		static u8* GetSymbolAddressAbsolute(Module* m, Symbol* sym);

		/**
		 * @brief Sets up relocation structures and calls a relocation routine.
		 * 
		 * @param srcAddr The address to write the relocation into.
		 * @param m Parent module of the relocation to process.
		 * @param r The relocation to process.
		 */
		static void DoRelocation(u8* srcAddr, Module* m, Relocation* r);

		/**
		 * @brief Converts a string to a standard RPM name hash.
		 * 
		 * @param name The string to convert.
		 * @return 32-bit FNV1a hash of the name.
		 */
		static RPM_NAMEHASH HashName(const char* name);
	};
}

#endif