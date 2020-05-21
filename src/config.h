#ifndef AE9565D5_8133_C640_1477_9B2F42A65F12
#define AE9565D5_8133_C640_1477_9B2F42A65F12

#include <stdbool.h>
#include <Python.h>
#include <datetime.h>

#define __YapicJson_STR(__v) #__v
#define YapicJson_STR(__v) __YapicJson_STR(__v)

#define YAPIC_JSON_VERSION_STR \
	YapicJson_STR(YAPIC_JSON_VERSION_MAJOR) "." \
	YapicJson_STR(YAPIC_JSON_VERSION_MINOR) "." \
	YapicJson_STR(YAPIC_JSON_VERSION_PATCH)

// see double-conversion.cc kMaxSignificantDigits
#define YAPIC_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS 772

#ifdef NDEBUG
#	ifndef YAPIC_JSON_ENCODER_BUFFER_SIZE
#		define YAPIC_JSON_ENCODER_BUFFER_SIZE 16384
#	endif
#	ifndef YAPIC_JSON_BYTES_DECODER_BUFFER_SIZE
#		define YAPIC_JSON_BYTES_DECODER_BUFFER_SIZE 16384
#	endif
#	ifndef YAPIC_JSON_CHUNK_BUFFER_SIZE
#		define YAPIC_JSON_CHUNK_BUFFER_SIZE 32768
#	endif
#else
#	define YAPIC_JSON_ENCODER_BUFFER_SIZE 10
#	define YAPIC_JSON_BYTES_DECODER_BUFFER_SIZE 10
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

#if defined(__has_cpp_attribute)
#	if __has_cpp_attribute(likely)
#		define YAPIC_LIKELY_ATTR [[likely]]
#		define YAPIC_UNLIKELY_ATTR [[unlikely]]
#	else
#		define YAPIC_LIKELY_ATTR
#		define YAPIC_UNLIKELY_ATTR
#	endif
#else
#	define YAPIC_LIKELY_ATTR
#	define YAPIC_UNLIKELY_ATTR
#endif

#if defined(__GNUC__) || defined(__clang__)
#	define LIKELY(x) __builtin_expect((x), 1)
#	define UNLIKELY(x) __builtin_expect((x), 0)
#else
#	define LIKELY(x) (x)
#	define UNLIKELY(x) (x)
#endif

#define IF_LIKELY(__cond) if (LIKELY((__cond))) YAPIC_LIKELY_ATTR
#define IF_UNLIKELY(__cond) if (UNLIKELY((__cond))) YAPIC_UNLIKELY_ATTR


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
