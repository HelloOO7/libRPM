/**
 * @file RPM_DllExport.h
 * @author Hello007
 * @brief Macros for importing/exporting libRPM publics. Import is assumed when included through RPM_Api.h.
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_DLLEXPORT_H
#define __RPM_DLLEXPORT_H

//https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
	#ifdef __GNUC__
		#define RPM_PUBLIC_IMPORT __attribute__ ((dllimport))
		#define RPM_PUBLIC_EXPORT __attribute__ ((dllexport))
	#else
		#define RPM_PUBLIC_IMPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
		#define RPM_PUBLIC_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
	#endif
	#define DLL_LOCAL
#else
	#if __GNUC__ >= 4
		#define RPM_PUBLIC_EXPORT __attribute__ ((visibility ("default")))
		#define RPM_PUBLIC_IMPORT RPM_PUBLIC_EXPORT
		#define RPM_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
	#else
		#define RPM_PUBLIC_EXPORT
		#define RPM_PUBLIC_IMPORT
		#define RPM_DLL_LOCAL
	#endif
#endif

#ifdef RPM_DLLAPI //import libRPM functions		
#define RPM_PUBLIC RPM_PUBLIC_IMPORT		
#else //building libRPM - export libRPM functions
#define RPM_PUBLIC RPM_PUBLIC_EXPORT
#endif

#endif