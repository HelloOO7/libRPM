#ifndef __RPM_MEMORYMANAGER_CPP
#define __RPM_MEMORYMANAGER_CPP

#include "exl_Types.h"
#include "exl_Allocator.h"

namespace exl {
	namespace heap {
		void* Allocator::FillAlloc(size_t size, u8 value) {
			void* data = Alloc(size);
			memset(data, value, size);
			return data;
		}

		#define GET_ALLOCATOR_REFERENCE(p) (reinterpret_cast<Allocator**>(p)[-1])

		Allocator* Allocator::GetAllocator(void* p) {
			return GET_ALLOCATOR_REFERENCE(p);
		}

		void* Allocator::ReallocStatic(void* p, size_t newSize) {
			if (p) {
				return GET_ALLOCATOR_REFERENCE(p)->Realloc(p, newSize);
			}
			return nullptr;
		}

		void Allocator::FreeStatic(void* p) {
			if (p) {
				GET_ALLOCATOR_REFERENCE(p)->Free(p);
			}
		}
	}
}

#endif