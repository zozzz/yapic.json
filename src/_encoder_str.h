#ifndef E286340C_7133_C640_1353_133ED6202CD2
#define E286340C_7133_C640_1353_133ED6202CD2

#include <type_traits>
#include "buffer.h"

namespace ZiboJson {

#define EncoderJT_AppendChar(ch) \
	*(out++) = (ch)

#define EncoderJT_ON_NULL() \
	buffer->cursor = out; \
	return;

#define EncoderJT_ON_ASCII() \
	EncoderJT_AppendChar(*(input++)); \
	continue

#define EncoderJT_ON_UNPRINTABLE() \
	EncoderJT_AppendChar('\\'); \
	EncoderJT_AppendChar('u'); \
	EncoderJT_AppendChar('0'); \
	EncoderJT_AppendChar('0'); \
	EncoderJT_AppendChar(HEX_CHAR(((*input) & 0xF0) >> 4)); \
	EncoderJT_AppendChar(HEX_CHAR(((*input) & 0x0F)));

#define STRING_ENCODER_FN_FROM_ASCII \
	std::is_same<CHIN, Py_UCS1>::value

#define STRING_ENCODER_FN_FROM_UNICODE \
	(std::is_same<CHIN, Py_UCS2>::value || std::is_same<CHIN, Py_UCS4>::value)

#define STRING_ENCODER_FN_TO_UNICODE \
	(std::is_base_of<Buffer<Py_UCS2>, BUFFER>::value || \
	 std::is_base_of<Buffer<Py_UCS4>, BUFFER>::value)

#define STRING_ENCODER_FN_TO_ASCII \
	std::is_base_of<Buffer<Py_UCS1>, BUFFER>::value

#define STRING_ENCODER_FN(from, x, to) \
	template<typename CHIN, typename BUFFER> \
	static inline void __encode_string(CHIN* input, BUFFER *buffer, \
		typename std::enable_if<(STRING_ENCODER_FN_FROM_ ## from) && (STRING_ENCODER_FN_TO_ ## to)>::type* = 0)

#define HEX_CHAR(idx) ("0123456789abcdef"[(idx)])


STRING_ENCODER_FN(ASCII, ->, ASCII) {
	#define EncoderJT_ON_DEL() \
		EncoderJT_AppendChar('\\'); \
		EncoderJT_AppendChar('u'); \
		EncoderJT_AppendChar('0'); \
		EncoderJT_AppendChar('0'); \
		EncoderJT_AppendChar('7'); \
		EncoderJT_AppendChar('F');


	typename BUFFER::Char* out = buffer->cursor;
	for (;;) {
		switch (*input) {
			#include "str_encode_jump.h"
			default:
				*(out++) = '\\';
				*(out++) = 'u';
				*(out++) = '0';
				*(out++) = '0';
				*(out++) = HEX_CHAR((*input & 0xF0) >> 4);
				*(out++) = HEX_CHAR((*input & 0x0F));
			break;
		}
		++input;
	}

	#undef EncoderJT_ON_DEL
}


STRING_ENCODER_FN(ASCII, ->, UNICODE) {
	#define EncoderJT_ON_DEL() \
		EncoderJT_AppendChar(*input);


	typename BUFFER::Char* out = buffer->cursor;
	for (;;) {
		switch (*input) {
			#include "str_encode_jump.h"
			default:
				*(out++) = '\\';
				*(out++) = 'u';
				*(out++) = '0';
				*(out++) = '0';
				*(out++) = HEX_CHAR((*input & 0xF0) >> 4);
				*(out++) = HEX_CHAR((*input & 0x0F));
			break;
		}
		++input;
	}


	#undef EncoderJT_ON_DEL
}


STRING_ENCODER_FN(UNICODE, ->, ASCII) {
	#define EncoderJT_ON_DEL() \
		EncoderJT_AppendChar('\\'); \
		EncoderJT_AppendChar('u'); \
		EncoderJT_AppendChar('0'); \
		EncoderJT_AppendChar('0'); \
		EncoderJT_AppendChar('7'); \
		EncoderJT_AppendChar('F');


	typename BUFFER::Char* out = buffer->cursor;
	for (;;) {
		switch (*input) {
			#include "str_encode_jump.h"
			default:
				*(out++) = '\\';
				*(out++) = 'u';
				if (*input > 0xFFFF) { 	// \uXXXX\uXXXX
					unsigned int higher = (*input >> 10) - 0x40;
					*(out++) = HEX_CHAR( 0xD );
					*(out++) = HEX_CHAR( 0x8 | ((higher >> 8) & 0x3) );
					*(out++) = HEX_CHAR( (higher >> 4) & 0xF );
					*(out++) = HEX_CHAR( higher & 0xF );

					*(out++) = '\\';
					*(out++) = 'u';
					*(out++) = HEX_CHAR( 0xD );
					*(out++) = HEX_CHAR( 0xC | ((*input >> 8) & 0x3) );
					*(out++) = HEX_CHAR( (*input >> 4) & 0xF );
					*(out++) = HEX_CHAR( *input & 0xF );
				} else { 					// \uXXXX
					*(out++) = HEX_CHAR( (*input >> 12) );
					*(out++) = HEX_CHAR( (*input >> 8) & 0xF );
					*(out++) = HEX_CHAR( (*input >> 4) & 0xF );
					*(out++) = HEX_CHAR( (*input) & 0xF );
				}
			break;
		}
		++input;
	}


	#undef EncoderJT_ON_DEL
}


STRING_ENCODER_FN(UNICODE, ->, UNICODE) {
	#define EncoderJT_ON_DEL() \
		EncoderJT_AppendChar(*input);


	typename BUFFER::Char* out = buffer->cursor;
	typename BUFFER::Char maxchar = buffer->maxchar;
	for (;;) {
		switch (*input) {
			#include "str_encode_jump.h"
			default:
				printf("DDDDD\n");
				if (maxchar < *input) {
					buffer->maxchar = maxchar = *input;
				}
				*(out++) = *(input++);
				continue;
		}
		++input;
	}


	#undef EncoderJT_ON_DEL
}

}

#endif /* E286340C_7133_C640_1353_133ED6202CD2 */
