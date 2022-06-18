#ifndef __EXL_MEMOPERATORS_CPP
#define __EXL_MEMOPERATORS_CPP

#include "exl_Allocator.h"
#include "exl_MemOperators.h"

void* operator new(size_t size) {
	while (true) {
		;
	}
}

void* operator new(size_t size, exl::heap::Allocator* allocator) {
	return allocator->Alloc(size);
}

void* operator new[](size_t size) {
	while (true) {
		;
	}
}

void* operator new[](size_t size, exl::heap::Allocator* allocator) {
	return allocator->Alloc(size);
}

void operator delete(void* p) {
	return exl::heap::Allocator::FreeStatic(p);
}

void operator delete(void* p, size_t sz) {
	return exl::heap::Allocator::FreeStatic(p);
}

void operator delete[](void* p) {
	return exl::heap::Allocator::FreeStatic(p);
}

void operator delete[](void* p, size_t sz) {
	return exl::heap::Allocator::FreeStatic(p);
}

#endif