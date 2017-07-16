#ifndef AE9565D5_8133_C640_1477_9B2F42A65F12
#define AE9565D5_8133_C640_1477_9B2F42A65F12

#include <stdbool.h>
#include <Python.h>
#include <datetime.h>

#if 0
#ifdef NDEBUG
#	define ZIBO_JSON_VERSION_STR \
		#ZIBO_JSON_VERSION_MAJOR "." \
		#ZIBO_JSON_VERSION_MINOR "." \
		#ZIBO_JSON_VERSION_PATH
#else
#	define ZIBO_JSON_VERSION_STR \
		#ZIBO_JSON_VERSION_MAJOR "." \
		#ZIBO_JSON_VERSION_MINOR "." \
		#ZIBO_JSON_VERSION_PATH "-debug"
#endif
#endif
#define ZIBO_JSON_VERSION_STR "1.0.0"

// see double-conversion.cc kMaxSignificantDigits
#define ZIBO_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS 772

#ifdef NDEBUG
#	ifndef ZIBO_JSON_ENCODER_BUFFER_SIZE
#		define ZIBO_JSON_ENCODER_BUFFER_SIZE 16384
#	endif
#	ifndef ZIBO_JSON_CHUNK_BUFFER_SIZE
#		define ZIBO_JSON_CHUNK_BUFFER_SIZE 1024
#	endif
#else
#	define ZIBO_JSON_ENCODER_BUFFER_SIZE 10
#	define ZIBO_JSON_CHUNK_BUFFER_SIZE 1
#endif

#ifdef __SSE4_2__
#	ifndef ZIBO_JSON_USE_SSE
#		define ZIBO_JSON_USE_SSE 1
#	endif
#else
#define ZIBO_JSON_USE_SSE 0
#endif

#ifdef _WIN32
#	include <Windows.h>
#	if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
#		define ZIBO_JSON_LITTLE_ENDIAN 1
#	else
#		define ZIBO_JSON_BIG_ENDIAN 1
#	endif
#else
#	if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#		define ZIBO_JSON_LITTLE_ENDIAN 1
#	else
#		define ZIBO_JSON_BIG_ENDIAN 1
#	endif
#endif


/**
 * Maximum recursion depth
 */
#define MAXIMUM_RECURSION_DEPTH 	1000
#define LLONG_MAX_LENGTH_IN_CHR 	20
#define DOUBLE_MAX_LENGTH_IN_CHR  	122

#ifdef NDEBUG
#	define ZiboLog(msg) ((void)0)
#else
#	include <stdio.h>
#	define ZiboLog(msg) printf(msg)
#endif

#if defined(__GNUC__) || defined(__clang__)
#	define EXPECT_TRUE(x) __builtin_expect((x), 1)
#	define EXPECT_FALSE(x) __builtin_expect((x), 0)
#else
#	define EXPECT_TRUE(x) (x) == 1
#	define EXPECT_FALSE(x) (x) == 0
#endif

#ifdef NDEBUG
#	define ZiboJson_Malloc   malloc
#	define ZiboJson_Realloc  realloc
#	define ZiboJson_Free     free
#else
#	define ZiboJson_Malloc   PyMem_MALLOC
#	define ZiboJson_Realloc  PyMem_REALLOC
#	define ZiboJson_Free     PyMem_FREE
#endif

#endif /* AE9565D5_8133_C640_1477_9B2F42A65F12 */
