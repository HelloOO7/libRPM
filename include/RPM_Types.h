/**
 * @file RPM_Types.h
 * @author Hello007
 * @brief Standard RPM typedefs and macros.
 * @version 0.1
 * @date 2022-01-17
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_TYPES_H
#define __RPM_TYPES_H

#ifdef __cplusplus

#include <cstdbool>
#include <cstddef>
#include <cstdio>
#include <cstdint>

#else

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#endif

/**
 * @brief Counts the number of elements in an array.
 */
#define NELEMS(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

/**
 * @brief Force-inlined function.
 */
#if defined(_MSC_VER)
#define INLINE inline __forceinline
#elif defined(__GNUC__)
#define INLINE inline __attribute__((always_inline))
#endif

#ifdef __GNUC__
#define RPM_PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define RPM_PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

typedef uint64_t u64;

typedef int64_t s64;

typedef uint32_t u32;

typedef int32_t s32;

typedef uint16_t u16;

typedef int16_t s16;

typedef uint8_t u8;

typedef int8_t s8;

#endif