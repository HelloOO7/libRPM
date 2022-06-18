/**
 * @file exl_DllExport.h
 * @author Hello007
 * @brief Macros for importing/exporting ExtLib publics.
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __EXL_DLLEXPORT_H
#define __EXL_DLLEXPORT_H

//https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
	#ifdef __GNUC__
		#define EXL_PUBLIC_IMPORT __attribute__ ((dllimport))
		#define EXL_PUBLIC_EXPORT __attribute__ ((dllexport))
	#else
		#define EXL_PUBLIC_IMPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
		#define EXL_PUBLIC_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
	#endif
	#define DLL_LOCAL
#else
	#if __GNUC__ >= 4
		#define EXL_PUBLIC_EXPORT __attribute__ ((visibility ("default")))
		#define EXL_PUBLIC_IMPORT EXL_PUBLIC_EXPORT
		#define EXL_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
	#else
		#define EXL_PUBLIC_EXPORT
		#define EXL_PUBLIC_IMPORT
		#define EXL_DLL_LOCAL
	#endif
#endif

#ifdef EXL_DLLAPI //import extlib functions		
#define EXL_PUBLIC EXL_PUBLIC_IMPORT		
#else //building extlib - export extlib functions
#define EXL_PUBLIC EXL_PUBLIC_EXPORT
#endif

#endif