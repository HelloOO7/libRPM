/**
 * @file RPM_EnumFlagOperators.h
 * @author Hello007
 * @brief WinAPI header for bitwise enum operations in C++.
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_ENUMFLAGOPERATORS_H
#define __RPM_ENUMFLAGOPERATORS_H

template <size_t S>
struct _ENUM_FLAG_INTEGER_FOR_SIZE;

template <>
struct _ENUM_FLAG_INTEGER_FOR_SIZE<1>
{
    typedef u8 type;
};

template <>
struct _ENUM_FLAG_INTEGER_FOR_SIZE<2>
{
    typedef u16 type;
};

template <>
struct _ENUM_FLAG_INTEGER_FOR_SIZE<4>
{
    typedef u32 type;
};

template <class T>
struct _ENUM_FLAG_SIZED_INTEGER
{
    typedef typename _ENUM_FLAG_INTEGER_FOR_SIZE<sizeof(T)>::type type;
};

#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) | ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) |= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) & ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) &= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a)); } \
inline ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) ^ ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) ^= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \

#endif