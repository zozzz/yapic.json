#ifndef AE9565D5_8133_C640_1477_9B2F42A65F12
#define AE9565D5_8133_C640_1477_9B2F42A65F12

#include <stdbool.h>
#include <Python.h>
#include <datetime.h>

#if 0
#ifdef NDEBUG
#	define YAPIC_JSON_VERSION_STR \
		#YAPIC_JSON_VERSION_MAJOR "." \
		#YAPIC_JSON_VERSION_MINOR "." \
		#YAPIC_JSON_VERSION_PATH
#else
#	define YAPIC_JSON_VERSION_STR \
		#YAPIC_JSON_VERSION_MAJOR "." \
		#YAPIC_JSON_VERSION_MINOR "." \
		#YAPIC_JSON_VERSION_PATH "-debug"
#endif
#endif
#define YAPIC_JSON_VERSION_STR "1.0.0"

// see double-conversion.cc kMaxSignificantDigits
#define YAPIC_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS 772

#ifdef NDEBUG
#	ifndef YAPIC_JSON_ENCODER_BUFFER_SIZE
#		define YAPIC_JSON_ENCODER_BUFFER_SIZE 16384
#	endif
#	ifndef YAPIC_JSON_CHUNK_BUFFER_SIZE
#		define YAPIC_JSON_CHUNK_BUFFER_SIZE 32768
#	endif
#else
#	define YAPIC_JSON_ENCODER_BUFFER_SIZE 10
#	define YAPIC_JSON_CHUNK_BUFFER_SIZE 1
#endif

#ifdef __SSE4_2__
#	ifndef YAPIC_JSON_USE_SSE
#		define YAPIC_JSON_USE_SSE 1
#	endif
#else
#define YAPIC_JSON_USE_SSE 0
#endif

#ifdef _WIN32
#	include <Windows.h>
#	if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
#		define YAPIC_JSON_LITTLE_ENDIAN 1
#	else
#		define YAPIC_JSON_BIG_ENDIAN 1
#	endif
#else
#	if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#		define YAPIC_JSON_LITTLE_ENDIAN 1
#	else
#		define YAPIC_JSON_BIG_ENDIAN 1
#	endif
#endif


/**
 * Maximum recursion depth
 */
#define MAXIMUM_RECURSION_DEPTH 	1000
#define LLONG_MAX_LENGTH_IN_CHR 	20
#define DOUBLE_MAX_LENGTH_IN_CHR  	122

#ifdef NDEBUG
#	define YapicLog(msg) ((void)0)
#else
#	include <stdio.h>
#	define YapicLog(msg) printf(msg)
#endif

#if defined(__GNUC__) || defined(__clang__)
#	define EXPECT_TRUE(x) __builtin_expect((x), 1)
#	define EXPECT_FALSE(x) __builtin_expect((x), 0)
#else
#	define EXPECT_TRUE(x) (x) == 1
#	define EXPECT_FALSE(x) (x) == 0
#endif

#ifdef NDEBUG
#	define YapicJson_Malloc   malloc
#	define YapicJson_Realloc  realloc
#	define YapicJson_Free     free
#else
#	define YapicJson_Malloc   PyMem_MALLOC
#	define YapicJson_Realloc  PyMem_REALLOC
#	define YapicJson_Free     PyMem_FREE
#endif

#endif /* AE9565D5_8133_C640_1477_9B2F42A65F12 */
