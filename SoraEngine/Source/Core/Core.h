/*
	Copyright © Sora Interactive. All rights reserved.
	Please read the LICENSE file 
*/

#pragma once

#ifndef SORA_PLATFORM_WINDOWS
#if defined(_WIN32) || defined(_WIN64)
#define SORA_PLATFORM_WINDOWS 1
#else
#define SORA_PLATFORM_WINDOWS 0
#endif
#endif

#if SORA_PLATFORM_WINDOWS
	#define SORA_HELPER_DLL_IMPORT __declspec(dllimport)
	#define SORA_HELPER_DLL_EXPORT __declspec(dllexport) 
#else
	#if __GNUC__ >= 4
		#define SORA_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
		#define SORA_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
	#else
		#define SORA_HELPER_DLL_IMPORT
		#define SORA_HELPER_DLL_EXPORT
	#endif
#endif

#ifdef SORA_DYNAMIC_LINK
	#ifdef SORA_BUILD_DLL
		#define SORA_API SORA_HELPER_DLL_EXPORT
	#else
		#define SORA_API SORA_HELPER_DLL_EXPORT
	#endif
#else // Static build → No import/export needed
	#define SORA_API
#endif

#if defined(_MSC_VER)
	#pragma warning(disable: 4251)
#endif

#define SORA_NO_DISCARD [[nodiscard]]