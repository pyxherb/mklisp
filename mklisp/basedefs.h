#ifndef _MKLISP_BASEDEFS_H_
#define _MKLISP_BASEDEFS_H_

#ifdef _MSC_VER

	#define MKLISP_DLLEXPORT __declspec(dllexport)
	#define MKLISP_DLLIMPORT __declspec(dllimport)
	#define MKLISP_FORCEINLINE __forceinline

#elif defined(__GNUC__)

	#define MKLISP_DLLEXPORT __attribute__((visilibity("default"))
	#define MKLISP_DLLIMPORT __attribute__((visilibity("default"))
	#define MKLISP_FORCEINLINE __attribute__((__always_inline__)) inline

#endif

#if MKLISP_BUILD_SHARED
	#if MKLISP_IS_BUILDING

		#if MKLISP_BUILD_SHARED
			#define MKLISP_API MKLISP_DLLEXPORT
		#endif

	#else

		#if MKLISP_BUILD_SHARED
			#define MKLISP_API MKLISP_DLLIMPORT
		#endif

	#endif
#else
	#define MKLISP_API
#endif

#endif
