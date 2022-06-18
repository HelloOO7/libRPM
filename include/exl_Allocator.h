/**
 * @file exl_Allocator.h
 * @author Hello007
 * @brief ExtLib dynamic memory allocator interface.
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __EXL_ALLOCATOR_H
#define __EXL_ALLOCATOR_H

namespace exl {
	namespace heap {
		class Allocator;
	}
}

#include "exl_Types.h"
#include "exl_MemOperators.h"
#include "exl_DllExport.h"
#include <cstring>

#define EXL_ALLOCATOR_BLOCK_END_REQUIRE exl::heap::Allocator* Allocator

namespace exl {
	namespace heap {
		class Allocator {
		public:
			/**
			 * @brief Allocates a block of bytes on the heap, the content of which may be undefined.
			 * 
			 * @param size Number of bytes to allocate.
			 * @return Pointer to a new memory block of 'size' bytes, or null if the allocation failed.
			 */
			EXL_PUBLIC virtual void* Alloc(size_t size) = 0;
			
			/**
			 * @brief Allocates a block of bytes on the heap, the content of which will be set to a defined value.
			 * 
			 * @param size Number of bytes to allocate.
			 * @param value The byte to fill the new memory block with.
			 * @return Pointer to a new memory block of 'size' bytes, or null if the allocation failed.
			 */
			EXL_PUBLIC void* FillAlloc(size_t size, u8 value);

			/**
			 * @brief Allocates a block of bytes on the heap, clearing all its content to 0x00.
			 * 
			 * @param size Number of bytes to allocate.
			 * @return Pointer to a zero-filled new memory block of 'size' bytes, or null if the allocation failed.
			 */
			EXL_PUBLIC INLINE void* CAlloc(size_t size) {
				return FillAlloc(size, 0);
			}

			/**
			 * @brief Resizes an allocated memory block to a new size.
			 * 
			 * @param p Pointer to the memory block to resize.
			 * @param newSize Desired new size of the memory block.
			 * @return 'p' if the block was shrunk or no additional allocation was needed, or a pointer to a new copy of the original block with the new size.
			 */
			EXL_PUBLIC virtual void* Realloc(void* p, size_t newSize) = 0;
			
			/**
			 * @brief Discards a memory block and frees its memory area for further allocation.
			 * 
			 * @param p Pointer to the memory block to discard.
			 */
			EXL_PUBLIC virtual void Free(void* p) = 0;

			/**
			 * @brief Gets the instance of the Allocator that allocated a memory block.
			 * 
			 * @param p Pointer to the memory block to get the Allocator of.
			 */
			EXL_PUBLIC static Allocator* GetAllocator(void* p);

			/**
			 * @brief Reallocates a MemoryManager-allocated block without an explicit MemoryManager instance.
			 * 
			 * See MemoryManager::Realloc
			 */
			EXL_PUBLIC static void* ReallocStatic(void* p, size_t newSize);
			/**
			 * @brief Discards a MemoryManager-allocated memory block without an explicit MemoryManager instance.
			 * 
			 * @param p Pointer to the memory block to discard.
			 */
			EXL_PUBLIC static void FreeStatic(void* p);
		};
	}
}

#endif