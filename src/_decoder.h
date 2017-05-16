#ifndef QDD73D15_6133_C640_1481_736A723E63B6
#define QDD73D15_6133_C640_1481_736A723E63B6

#include <limits>

#include "../libs/double-conversion/double-conversion/double-conversion.h"
#include "../libs/double-conversion/double-conversion/strtod.h"
#include "config.h"
#include "buffer.h"


#define Decoder_Error(msg) \
	PyErr_Format(DecodeError, msg " at XYZ pos."); \
	return NULL

#define Decoder_ErrorFormat(msg, ...) \
	PyErr_Format(DecodeError, msg " at XYZ pos.", __VA_ARGS__); \
	return NULL

#define Decoder_FN(name) \
	inline PyObject* name()

#define Decoder_IsWhiteSpace(ch) \
	((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r')

#define Decoder_EatWhiteSpace() \
	if (Decoder_IsWhiteSpace(*cursor)) { \
		CHIN wsc = *(cursor++); \
		while (Decoder_IsWhiteSpace(wsc)) { wsc = *(cursor++); } \
	}

#define Decoder_EnsureCapacity(required) \
	if ((required) > buffer.end - buffer.cursor && buffer.EnsureCapacity(required) == false) { \
		return NULL; \
	} else { \
		out = buffer.cursor; \
	}

// {
// 	CHIN wsc = *input;
// 	while (Decoder_IsWhiteSpace(wsc)) {
// 		wsc = *(++input);
// 	}
// }

namespace ZiboJson {
using namespace double_conversion;

static const StringToDoubleConverter& NumberParser() {
	static StringToDoubleConverter converter(
		StringToDoubleConverter::ALLOW_TRAILING_JUNK | StringToDoubleConverter::ALLOW_TRAILING_SPACES,
		0.0, Py_HUGE_VAL, NULL, NULL);
  	return converter;
}

template<typename CHIN, typename BUFF>
class Decoder {
	public:
		typedef typename BUFF::Char CHOUT;

		PyObject* objectHook;
		PyObject* parseFloat;
		bool parseDate;

		Decoder(CHIN* data, size_t length)
			: start(data), end(data + length), cursor(data) {
			Decoder_EatWhiteSpace();
		}

		Decoder_FN(ReadValue) {
			switch (*cursor) {
				case '"':
					return ReadString();
				break;

				case '-':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					return ReadNumber();
				break;

				case '{':
					return ReadDict();
				break;

				case '[':
					return ReadList();
				break;

				case 't':
					return ReadTrue();
				break;

				case 'f':
					return ReadFalse();
				break;

				case 'n':
					return ReadNull();
				break;

				case 'I':
					return ReadPInfinity();
				break;

				case 'N':
					return ReadNaN();
				break;

				default:
					Decoder_ErrorFormat(ZiboJson_Err_UnexpectedChar, *cursor);
				break;
			}

			return NULL;
		}

	private:
		BUFF buffer;
		char floatBuffer[ZIBO_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS + 10];
		const CHIN* start;
		const CHIN* end;
		CHIN* cursor;


#if 0

		#define ReadString_Append(ch) \
			*(out++) = (ch)

		#define ReadUnicodeEscapePart(output) \
			++i; \
			if (chunk[i] >= '0' && chunk[i] <= '9')  { \
				output = (output << 4) + (chunk[i] - '0'); \
			} else if (chunk[i] >= 'a' && chunk[i] <= 'f') { \
				output = (output << 4) + (chunk[i] - 'a' + 10); \
			} else if (chunk[i] >= 'A' && chunk[i] <= 'F') { \
				output = (output << 4) + (chunk[i] - 'A' + 10); \
			} else { \
				if (chunk[i] == '\0') { Decoder_Error(ZiboJson_Err_UnexpectedEnd); } \
				Decoder_Error(ZiboJson_Err_UnexpectedCharInUnicodeEscape); \
			}


		#define ReadUnicodeEscape(output) \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output);

		Decoder_FN(ReadString) {
			register CHIN* chunk = ++cursor;
			register CHOUT* out = buffer.cursor;
			register size_t i = 0;
			register CHOUT maxchar = buffer.maxchar;

			for (;;) {
				while (i < ZIBO_JSON_DECODER_BUFFER_SIZE) {
					if (chunk[i] == '"') {
						cursor += i;
						buffer.cursor = out;
						return buffer.NewString();
					} else if (chunk[i] == '\\') {
						switch (chunk[++i]) {
							case 'b': ReadString_Append('\b'); break;
							case 'f': ReadString_Append('\f'); break;
							case 'n': ReadString_Append('\n'); break;
							case 'r': ReadString_Append('\r'); break;
							case 't': ReadString_Append('\t'); break;
							case '"': ReadString_Append('"'); break;
							case '\\': ReadString_Append('\\'); break;
							case '/': ReadString_Append('/'); break;
							case 'u': {
								CHOUT ucs_pt1 = 0;
								ReadUnicodeEscape(ucs_pt1);

								if ((ucs_pt1 & 0xFC00) == 0xD800) {
									if (chunk[++i] == '\\' && chunk[++i] == 'u' ) {
										CHOUT ucs_pt2 = 0;
										ReadUnicodeEscape(ucs_pt2);

										if ((ucs_pt2 & 0xFC00) == 0xDC00) {
											ucs_pt1 = 0x10000 + (((ucs_pt1 - 0xD800) << 10) | (ucs_pt2 - 0xDC00));
										} else {
											Decoder_Error(ZiboJson_Err_UnpairedHighSurrogate);
										}
									} else {
										Decoder_Error(ZiboJson_Err_UnpairedHighSurrogate);
									}
								} else if ((ucs_pt1 & 0xFC00) == 0xDC00) {
									Decoder_Error(ZiboJson_Err_UnpairedLowSurrogate);
								}

								ReadString_Append(ucs_pt1);
								if (ucs_pt1 > maxchar) {
									buffer.maxchar = maxchar = ucs_pt1;
								}
							} break;

							case '\0':
								Decoder_Error(ZiboJson_Err_UnexpectedEnd);
							break;

							default:
								Decoder_Error(ZiboJson_Err_InvalidEscape);
							break;
						}
						++i;
					} else {
						if (sizeof(CHIN) > 1 && chunk[i] > maxchar) {
							buffer.maxchar = maxchar = chunk[i];
						}
						ReadString_Append(chunk[i++]);
					}
				}

				cursor += i;
				chunk = cursor;
				buffer.cursor = out;
				Decoder_EnsureCapacity(ZIBO_JSON_DECODER_BUFFER_SIZE);
			}

			return NULL;
		}

#elif 1


		#define ReadString_Append(ch) \
			*(out++) = (ch)

		#define ReadUnicodeEscapePart(output) \
			++inp; \
			if (*inp >= '0' && *inp <= '9')  { \
				output = (output << 4) + (*inp - '0'); \
			} else if (*inp >= 'a' && *inp <= 'f') { \
				output = (output << 4) + (*inp - 'a' + 10); \
			} else if (*inp >= 'A' && *inp <= 'F') { \
				output = (output << 4) + (*inp - 'A' + 10); \
			} else { \
				if (*inp == '\0') { Decoder_Error(ZiboJson_Err_UnexpectedEnd); } \
				Decoder_Error(ZiboJson_Err_UnexpectedCharInUnicodeEscape); \
			}

		#define ReadUnicodeEscape(output) \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output);

		Decoder_FN(ReadString) {
			register CHIN* inp = ++cursor;
			register CHOUT* out = buffer.cursor;
			register CHOUT* outEnd = buffer.end;
			register CHOUT maxchar = buffer.maxchar;

			for (;;) {
				while (out < outEnd) {
					if (*inp == '"') {
						cursor = ++inp;
						buffer.cursor = out;
						PyObject* str = buffer.NewString();
						buffer.cursor = buffer.start;
						buffer.maxchar = 127;
						return str;
					} else if (*inp == '\\') {
						switch (*(++inp)) {
							case 'b': ReadString_Append('\b'); break;
							case 'f': ReadString_Append('\f'); break;
							case 'n': ReadString_Append('\n'); break;
							case 'r': ReadString_Append('\r'); break;
							case 't': ReadString_Append('\t'); break;
							case '"': ReadString_Append('"'); break;
							case '\\': ReadString_Append('\\'); break;
							case '/': ReadString_Append('/'); break;
							case 'u': {
								CHOUT ucs_pt1 = 0;
								ReadUnicodeEscape(ucs_pt1);

								if ((ucs_pt1 & 0xFC00) == 0xD800) {
									if (*(++inp) == '\\' && *(++inp) == 'u' ) {
										CHOUT ucs_pt2 = 0;
										ReadUnicodeEscape(ucs_pt2);
										if ((ucs_pt2 & 0xFC00) == 0xDC00) {
											ucs_pt1 = 0x10000 + (((ucs_pt1 - 0xD800) << 10) | (ucs_pt2 - 0xDC00));
										} else {
											Decoder_Error(ZiboJson_Err_UnpairedHighSurrogate);
										}
									} else {
										Decoder_Error(ZiboJson_Err_UnpairedHighSurrogate);
									}
								} else if ((ucs_pt1 & 0xFC00) == 0xDC00) {
									Decoder_Error(ZiboJson_Err_UnpairedLowSurrogate);
								}

								ReadString_Append(ucs_pt1);
								if (ucs_pt1 > maxchar) {
									buffer.maxchar = maxchar = ucs_pt1;
								}
							} break;

							case '\0':
								Decoder_Error(ZiboJson_Err_UnexpectedEnd);
							break;

							default:
								Decoder_Error(ZiboJson_Err_InvalidEscape);
							break;
						}
						++inp;
					} else {
						if (sizeof(CHIN) > 1 && *inp > maxchar) {
							buffer.maxchar = maxchar = *inp;
						}

						ReadString_Append(*(inp++));
					}
				}

				buffer.cursor = out;
				if (EXPECT_TRUE(buffer.EnsureCapacity(ZIBO_JSON_DECODER_BUFFER_SIZE))) {
					out = buffer.cursor;
					outEnd = buffer.end;
				} else {
					return NULL;
				}
			}

			return NULL;
		}

#elif 0

		#define ReadString_Append(ch) \
			*(out++) = (ch)

		#define ReadUnicodeEscapePart(output) \
			++inp; \
			if (*inp >= '0' && *inp <= '9')  { \
				output = (output << 4) + (*inp - '0'); \
			} else if (*inp >= 'a' && *inp <= 'f') { \
				output = (output << 4) + (*inp - 'a' + 10); \
			} else if (*inp >= 'A' && *inp <= 'F') { \
				output = (output << 4) + (*inp - 'A' + 10); \
			} else { \
				if (*inp == '\0') { Decoder_Error(ZiboJson_Err_UnexpectedEnd); } \
				Decoder_Error(ZiboJson_Err_UnexpectedCharInUnicodeEscape); \
			}


		#define ReadUnicodeEscape(output) \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output);

		Decoder_FN(ReadString) {
			register CHIN* inp = ++cursor;
			register CHOUT* out = buffer.cursor;
			register CHOUT* outEnd = buffer.end;
			CHOUT maxchar = buffer.maxchar;

			for (;;) {
				if (*inp == '"') {
					cursor = inp;
					buffer.cursor = out;
					return buffer.NewString();
				} else if (*inp == '\\') {
					switch (*(++inp)) {
						case 'b': ReadString_Append('\b'); break;
						case 'f': ReadString_Append('\f'); break;
						case 'n': ReadString_Append('\n'); break;
						case 'r': ReadString_Append('\r'); break;
						case 't': ReadString_Append('\t'); break;
						case '"': ReadString_Append('"'); break;
						case '\\': ReadString_Append('\\'); break;
						case '/': ReadString_Append('/'); break;
						case 'u': {
							CHOUT ucs_pt1 = 0;
							ReadUnicodeEscape(ucs_pt1);

							if ((ucs_pt1 & 0xFC00) == 0xD800) {
								if (*(++inp) == '\\' && *(++inp) == 'u' ) {
									CHOUT ucs_pt2 = 0;
									ReadUnicodeEscape(ucs_pt2);
									if ((ucs_pt2 & 0xFC00) != 0xDC00) {
										Decoder_Error(ZiboJson_Err_UnpairedHighSurrogate);
									}

									ucs_pt1 = 0x10000 + (((ucs_pt1 - 0xD800) << 10) | (ucs_pt2 - 0xDC00));
								} else {
									Decoder_Error(ZiboJson_Err_UnpairedHighSurrogate);
								}
							} else if ((ucs_pt1 & 0xFC00) == 0xDC00) {
								Decoder_Error(ZiboJson_Err_UnpairedLowSurrogate);
							}

							ReadString_Append(ucs_pt1);
							if (ucs_pt1 > maxchar) {
								buffer.maxchar = maxchar = ucs_pt1;
							}
						} break;

						case '\0':
							Decoder_Error(ZiboJson_Err_UnexpectedEnd);
						break;

						default:
							Decoder_Error(ZiboJson_Err_InvalidEscape);
						break;
					}
					++inp;
				} else {
					ReadString_Append(*(inp++));
				}

				if (out >= outEnd) {
					buffer.cursor = out;
					if (EXPECT_TRUE(buffer.EnsureCapacity(ZIBO_JSON_DECODER_BUFFER_SIZE))) {
						out = buffer.cursor;
						outEnd = buffer.end;
					} else {
						return NULL;
					}
				}

			}

			return NULL;
		}


#endif

		#define Decoder_IsNumber(ch) \
			((ch) >= '0' && (ch) <= '9')

		#define __return_float() \
			if (sizeof(CHIN) == 1) { \
				return PyFloat_FromDouble(_Py_dg_strtod((char*) cursor, (char**) &cursor)); \
			} else if (sizeof(CHIN) == sizeof(wchar_t)) { \
				return PyFloat_FromDouble(wcstod((wchar_t*) cursor, (wchar_t**) &cursor)); \
			} else { \
				int length = input - cursor; \
				char* tmp = (char*) ZiboJson_Malloc(length + 1); \
				if (tmp) { \
					int i = 0; \
					while (i < length) { \
						tmp[i] = cursor[i++]; \
					} \
					tmp[length] = '\0'; \
					cursor += length; \
					PyObject* result = PyFloat_FromDouble(_Py_dg_strtod(tmp, NULL)); \
					ZiboJson_Free(tmp); \
					return result; \
				} else { \
					return PyErr_NoMemory(); \
				} \
			}


		Decoder_FN(ReadNumber) {
			register CHIN* input = cursor;
			register char* floatCursor = floatBuffer;
			// if parsing int, contains int value
			// if parsing float, contains exponent value
			register unsigned long long temp = 0;
			register int significantDigits = 0;
			int exponent = 0;
			bool isNegative = false;
			bool nonzeroDigitDropped = false;

			if (*input == '-') {
				input++;
				if (*input == 'I') {
					cursor = input;
					return ReadNInfinity();
				}
				isNegative = true;
			}

			if (*input > '0' && *input <= '9') { // int number only starts with 1-9 numbers
				floatCursor[significantDigits++] = *input;
				temp = (temp * 10) + (*(input++) - '0');
			} else if (*(input + 1) == '.') { // 0.
				++input;
				goto parse_float;
			} else if (*(input + 1) == 'e' || *(input + 1) == 'E') { // 0[eE]
				++input;
				goto parse_exponent;
			} else {
				cursor = ++input;
				return PyLong_FromLong(0);
			}

			while (Decoder_IsNumber(*input)) {
				if (temp < (unsigned long long) (LLONG_MAX)) {
					floatCursor[significantDigits++] = *input;
					temp = (temp * 10) + (*(input++) - '0');
				} else if (significantDigits < ZIBO_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS) {
					floatCursor[significantDigits++] = *(input++);
				} else {
					nonzeroDigitDropped = nonzeroDigitDropped || *(input - 1) != '0';
				}
			}

			if (*input == '.') {
parse_float:
				if (temp == 0) {
					while (*(++input) == '0') {
						--exponent;
					}
					if (*input == '\0') {
						return PyFloat_FromDouble(0.0);
					}
				} else {
					++input;
				}

				// In JSON must have one digit after dot
				if (Decoder_IsNumber(*input)) {
					--exponent;
					if (significantDigits < ZIBO_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS) {
						floatCursor[significantDigits++] = *(input++);
					} else {
						nonzeroDigitDropped = nonzeroDigitDropped || *(input - 1) != '0';
					}

					while (Decoder_IsNumber(*input)) {
						--exponent;
						if (significantDigits < ZIBO_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS) {
							floatCursor[significantDigits++] = *(input++);
						} else {
							nonzeroDigitDropped = nonzeroDigitDropped || *(input - 1) != '0';
						}
					}

					if (*input == 'e' || *input == 'E') {
						goto parse_exponent;
					} else {
						goto return_float;
					}
				} else {
					Decoder_Error(ZiboJson_Err_UnexpectedCharInNumber);
				}
			}

			if (*input == 'e' || *input == 'E') {
parse_exponent:
				bool exponentIsNegative = false;
				int e = 0;
				++input;

				if (*input == '-') {
					exponentIsNegative = true;
					++input;
				} else if (*input == '+') {
					++input;
				}

				if (Decoder_IsNumber(*input)) {
					e = (e * 10) + (*(input++) - '0');
					while (Decoder_IsNumber(*input)) {
						e = (e * 10) + (*(input++) - '0');
					}
					exponent += exponentIsNegative ? -e : e;
					goto return_float;
				} else {
					Decoder_Error(ZiboJson_Err_UnexpectedCharInNumber);
				}
			}

			cursor = input;
			if (isNegative) {
				if (temp > (unsigned long long) (LLONG_MAX) + 1) {
					Decoder_Error(ZiboJson_Err_NumberIsTooBig);
				}
				temp = -temp;
			} else if (temp > (unsigned long long) (LLONG_MAX)) {
				Decoder_Error(ZiboJson_Err_NumberIsTooBig);
			}
			return PyLong_FromLongLong(static_cast<long long>(temp));

return_float:
			cursor = input;
			if (nonzeroDigitDropped) {
				floatCursor[significantDigits++] = '1';
				--temp;
			}

			double result = Strtod(Vector<const char>(floatBuffer, significantDigits), exponent);
			return PyFloat_FromDouble((isNegative ? -result : result));
		}

		Decoder_FN(ReadList) {
			register PyObject* list = PyList_New(0);
			register PyObject* item;
			if (list == NULL) {
				return NULL;
			}

			++cursor;
			Decoder_EatWhiteSpace();

			if (*cursor == ']') {
				++cursor;
				return list;
			}

			for (;;) {
				if ((item = ReadValue())) {
					PyList_Append(list, item);
					Py_DECREF(item);
					Decoder_EatWhiteSpace();

					if (*cursor == ',') {
						++cursor;
						Decoder_EatWhiteSpace();
					} else if (*cursor == ']') {
						++cursor;
						return list;
					} else {
						Py_DECREF(list);
						if (*cursor == '\0') {
							Decoder_Error(ZiboJson_Err_UnexpectedEnd);
						} else {
							Decoder_Error(ZiboJson_Err_UnexpectedCharInList);
						}
					}
				} else {
					Py_DECREF(list);
					return NULL;
				}
			}
		}

		Decoder_FN(ReadDict) {
			return NULL;
		}

		Decoder_FN(ReadTrue) {
			if (cursor[1] == 'r' && cursor[2] == 'u' && cursor[3] == 'e') {
				cursor += 4;
				Py_RETURN_TRUE;
			}
			Decoder_Error(ZiboJson_Err_UnexpectedCharInTrue);
		}

		Decoder_FN(ReadFalse) {
			if (cursor[1] == 'a' && cursor[2] == 'l' && cursor[3] == 's' && cursor[4] == 'e') {
				cursor += 5;
				Py_RETURN_FALSE;
			}
			Decoder_Error(ZiboJson_Err_UnexpectedCharInFalse);
		}

		Decoder_FN(ReadNInfinity) {
			if (cursor[1] == 'n' && cursor[2] == 'f' && cursor[3] == 'i' && cursor[4] == 'n' && cursor[5] == 'i' && cursor[6] == 't' && cursor[7] == 'y') {
				cursor += 8;
				return PyFloat_FromDouble(-Py_HUGE_VAL);
			}
			Decoder_Error(ZiboJson_Err_UnexpectedCharInInfinity);
		}

		Decoder_FN(ReadPInfinity) {
			if (cursor[1] == 'n' && cursor[2] == 'f' && cursor[3] == 'i' && cursor[4] == 'n' && cursor[5] == 'i' && cursor[6] == 't' && cursor[7] == 'y') {
				cursor += 8;
				return PyFloat_FromDouble(Py_HUGE_VAL);
			}
			Decoder_Error(ZiboJson_Err_UnexpectedCharInInfinity);
		}

		Decoder_FN(ReadNull) {
			if (cursor[1] == 'u' && cursor[2] == 'l' && cursor[3] == 'l') {
				cursor += 4;
				Py_RETURN_NONE;
			}
			Decoder_Error(ZiboJson_Err_UnexpectedCharInNull);
		}

		Decoder_FN(ReadNaN) {
			if (cursor[1] == 'a' && cursor[2] == 'N') {
				cursor += 3;
				Py_RETURN_NAN;
			}
			Decoder_Error(ZiboJson_Err_UnexpectedCharInNaN);
		}
};

} /* end namespace ZiboJson */


#endif /* QDD73D15_6133_C640_1481_736A723E63B6 */
