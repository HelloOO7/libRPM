/**
 * @file RPM_CpuUtil.h
 * @author Hello007
 * @brief Utility for encoding CPU instructions.
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_CPUUTIL_H
#define __RPM_CPUUTIL_H

#include "RPM_Types.h"
#include "RPM_Control.h"
#include "RPM_ARMAsm.h"

namespace rpm {
	namespace cpu {
		/**
		 * @brief Intermediate structure for relocation operations. For internal use only.
		 */
		struct CpuRelRequest {
			/**
			 * @brief The relocation procedure to use.
			 */
			rpm::RelTargetType 	Type;
			/**
			 * @brief Absolute address to write the relocation data at.
			 */
			u8* 				Source;
			/**
			 * @brief Absolute address to point the relocation towards.
			 */
			u8* 	   			Target;
			/**
			 * @brief The parent symbol of the relocation target.
			 */
			rpm::Symbol* 	    Symbol;
		};

		/**
		 * @brief A relocation procedure.
		 */
		typedef void (*RelFunction)(CpuRelRequest*);

		class CpuUtil {
		private:
			static void Reloc_OFFSET(CpuRelRequest* req);
			static void Reloc_THUMB_BL(CpuRelRequest* req);
			static void Reloc_ARM_BL(CpuRelRequest* req);
			static void Reloc_THUMB_B(CpuRelRequest* req);
			static void Reloc_ARM_B(CpuRelRequest* req);
			static void Reloc_FULL_COPY(CpuRelRequest* req);
			static void Reloc_THUMB_B_SAFESTACK(CpuRelRequest* req);
			static void Reloc_OFFSET_REL31(CpuRelRequest* req);

			static const RelFunction REL_FUNCTIONS[];
		public:

			/**
			 * @brief Performs a relocation.
			 * 
			 * @param req Request parameter containing relocation info.
			 */
			static void ProcessRelRequest(CpuRelRequest* req);
		protected:
			/**
			 * @brief Writes a 16-bit value to an aligned memory location.
			 * 
			 * @param dest The address to write to.
			 * @param value The value to write.
			 */
			INLINE static void Write16(u8* dest, u16 value) {
				*reinterpret_cast<u16*>(dest) = value; //WARNING: Dest must be WORD-aligned
			}

			/**
			 * @brief Reads a 32-bit value from an aligned memory location.
			 * 
			 * @param src The address to read from.
			 * @return The value at src interpreted as a 32-bit integer.
			 */
			INLINE static u32 Read32(u8* src) {
				return *reinterpret_cast<u32*>(src); //WARNING: Src must be DWORD-aligned
			}

			/**
			 * @brief Writes a 32-bit value to an aligned memory location.
			 * 
			 * @param dest The address to write to.
			 * @param value The value to write.
			 */
			INLINE static void Write32(u8* dest, u32 value) {
				*reinterpret_cast<u32*>(dest) = value; //WARNING: Dest must be DWORD-aligned
			}

			/**
			 * @brief Writes a 16-bit value to a byte stream and advances the stream by 2 bytes.
			 * 
			 * @param dest The stream to write to.
			 * @param value The value to write.
			 */
			INLINE static void StreamWrite16(u8** dest, u16 value) {
				Write16(*dest, value);
				*dest += sizeof(u16);
			}

			/**
			 * @brief Writes a 32-bit value to a byte stream and advances the stream by 4 bytes.
			 * 
			 * @param dest The stream to write to.
			 * @param value The value to write.
			 */
			INLINE static void StreamWrite32(u8** dest, u32 value) {
				Write32(*dest, value);
				*dest += sizeof(u32);
			}

			/**
			 * @brief Checks if the Thumb bit of an address is set.
			 * 
			 * @param addr The address to evaluate.
			 * @return True if the Thumb bit is set to 1.
			 */
			INLINE static bool IsAddrThumb(void* addr) {
				return reinterpret_cast<size_t>(addr) & 1;
			}
		};
	}
}

#endif