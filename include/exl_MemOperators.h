/**
 * @file exl_MemOperators.h
 * @author Hello007
 * @brief ExtLib allocator new and delete operator header.
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __RPM_MEMOPERATORS_H
#define __RPM_MEMOPERATORS_H

#include "exl_Allocator.h"
#include "exl_DllExport.h"

/**
 * @brief Global new operator to forbid non-Allocator dynamic allocation.
 */
EXL_PUBLIC void* operator new(size_t size);

/**
 * @brief Global new operator for Allocator-based allocation.
 */
EXL_PUBLIC void* operator new(size_t size, exl::heap::Allocator* allocator);

/**
 * @brief Global new array operator to forbid non-Allocator dynamic allocation.
 */
EXL_PUBLIC void* operator new[](size_t size);

/**
 * @brief Global new array operator for Allocator-based allocation.
 */
EXL_PUBLIC void* operator new[](size_t size, exl::heap::Allocator* allocator);

/**
 * @brief Global delete operator for Allocator-based allocation.
 */
EXL_PUBLIC void operator delete(void* p);

/**
 * @brief Global delete operator for Allocator-based allocation to satisfy compilers that require the second parameter.
 */
EXL_PUBLIC void operator delete(void* p, size_t sz);

/**
 * @brief Global array delete operator for Allocator-based allocation.
 */
EXL_PUBLIC void operator delete[](void* p);

/**
 * @brief Global array delete operator for Allocator-based allocation to satisfy compilers that require the second parameter.
 */
EXL_PUBLIC void operator delete[](void* p, size_t sz);

#endif