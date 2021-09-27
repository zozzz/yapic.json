#ifndef QDD73D15_6133_C640_1481_736A723E63B6
#define QDD73D15_6133_C640_1481_736A723E63B6

#include <limits>

#include "../libs/double-conversion/double-conversion/double-conversion.h"
#include "../libs/double-conversion/double-conversion/strtod.h"
#include "config.h"
#include "buffer.h"

#undef min
#undef max

#define Decoder_Error(msg) \
	PyErr_Format(Module::State()->DecodeError, msg " at position: %ld.", cursor - inputStart)

#define Decoder_ErrorFormat(msg, ...) \
	PyErr_Format(Module::State()->DecodeError, msg " at position: %ld.", __VA_ARGS__, cursor - inputStart)

#define Decoder_FN(name) \
	inline PyObject* name(CHIN* cursor, CHIN** cursorOut)

#define Decoder_IsWhiteSpace(ch) \
	((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r')

#define Decoder_EatWhiteSpace(cursor) \
	while (Decoder_IsWhiteSpace(*(cursor))) { ++(cursor); }


#if PY_VERSION_HEX < 0x03060100
	static const int _days_in_month[] = {0,
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

#	define DateParser_IsLeapYear(Y) \
		(Y % 4 == 0 && (Y % 100 != 0 || Y % 400 == 0))

#	define DateParser_DayInMonth(Y, M) \
		(M == 2 && DateParser_IsLeapYear(Y) ? 29 : _days_in_month[M])

#	define DateParser_CheckDate(Y, M, D) \
		if (Y < 1 || Y > 9999) { PyErr_Format(PyExc_ValueError, "year %i is out of range", Y); return NULL; } \
		if (M < 1 || M > 12) { PyErr_SetString(PyExc_ValueError, "month must be in 1..12"); return NULL; } \
		if (D < 1 || D > DateParser_DayInMonth(Y, M)) { PyErr_SetString(PyExc_ValueError, "day is out of range for month"); return NULL; }

#	define DateParser_CheckTime(h, m, s, f) \
		if (h < 0 || h > 23) { PyErr_SetString(PyExc_ValueError, "hour must be in 0..23"); return NULL; } \
		if (m < 0 || m > 59) { PyErr_SetString(PyExc_ValueError, "minute must be in 0..59"); return NULL; } \
		if (s < 0 || s > 59) { PyErr_SetString(PyExc_ValueError, "second must be in 0..59"); return NULL; } \
		if (f < 0 || f > 999999) { PyErr_SetString(PyExc_ValueError, "microsecond must be in 0..999999"); return NULL; }
#else
#	define DateParser_CheckDate(Y, M, D) ((void)0)
#	define DateParser_CheckTime(h, m, s, f) ((void)0)
#endif


namespace Yapic { namespace Json {
using namespace double_conversion;

#include "str_decode_table.h"

#define ReadUnicodeEscapePart(output) \
	++cursor; \
	if (*cursor >= '0' && *cursor <= '9')  { \
		output = (output << 4) + (*cursor - '0'); \
	} else if (*cursor >= 'a' && *cursor <= 'f') { \
		output = (output << 4) + (*cursor - 'a' + 10); \
	} else if (*cursor >= 'A' && *cursor <= 'F') { \
		output = (output << 4) + (*cursor - 'A' + 10); \
	} else { \
		if (*cursor == '\0') { Decoder_Error(YapicJson_Err_UnexpectedEnd); } \
		else { Decoder_Error(YapicJson_Err_UnexpectedCharInUnicodeEscape); } \
		return false; \
	}

#define ReadUnicodeEscape(output) \
		ReadUnicodeEscapePart(output); \
		ReadUnicodeEscapePart(output); \
		ReadUnicodeEscapePart(output); \
		ReadUnicodeEscapePart(output);

#define BR_IS_UTF8_ASCII(ch) ((ch) < 0x80)
#define BR_IS_UTF8_LENGTH_2(ch) ((ch) < 0xE0)
#define BR_IS_UTF8_LENGTH_3(ch) ((ch) < 0xF0)
#define BR_IS_UTF8_LENGTH_4(ch) ((ch) < 0xF5)
#define BR_IS_UTF8_CONT(ch) ((ch) >= 0x80 && (ch) < 0xC0)


template<typename CHIN, typename CHOUT, typename BUFF>
class StringReader {
	public:
		static inline PyObject* Read(CHIN *&cursor, CHIN **cursorOut, const CHIN * const inputStart, const CHIN * const inputEnd, BUFF &buffer) {
			CHOUT maxchar = 127;
			CHOUT escaped;

			while (cursor < inputEnd) {
				if (*cursor == '"') {
					goto success;
				} else if (*cursor == '\\') {
					IF_LIKELY (ReadEscapeSeq(cursor, inputStart, inputEnd, escaped) && buffer.AppendChar(escaped)) {
						maxchar |= escaped;
						++cursor;
					} else {
						return NULL;
					}
				} else {
					CHIN* sliceBegin = cursor;

					do {
						maxchar |= *cursor;
					} while (++cursor < inputEnd && *cursor != '\\' && *cursor != '"');

					if (!buffer.AppendSlice(sliceBegin, cursor - sliceBegin)) {
						return NULL;
					}
				}
			}

			Decoder_Error(YapicJson_Err_UnexpectedEnd);
			return NULL;

			success:
				*cursorOut = ++cursor;
				return buffer.NewString(maxchar);
		}

		static inline bool ReadEscapeSeq(CHIN *&cursor, const CHIN * const inputStart, const CHIN * const inputEnd, CHOUT &result) {
			switch (*(++cursor)) {
				case 'b': result = '\b'; break;
				case 'f': result = '\f'; break;
				case 'n': result = '\n'; break;
				case 'r': result = '\r'; break;
				case 't': result = '\t'; break;
				case '"': result = '"'; break;
				case '\\': result = '\\'; break;
				case '/': result = '/'; break;
				case 'u': {
					result = 0;
					ReadUnicodeEscape(result);

					if ((result & 0xFC00) == 0xD800) {
						if (*(++cursor) == '\\' && *(++cursor) == 'u' ) {
							CHOUT ucs_pt2 = 0;
							ReadUnicodeEscape(ucs_pt2);
							if ((ucs_pt2 & 0xFC00) == 0xDC00) {
								result = 0x10000 + (((result - 0xD800) << 10) | (ucs_pt2 - 0xDC00));
							} else {
								Decoder_Error(YapicJson_Err_UnpairedHighSurrogate);
								return false;
							}
						} else {
							Decoder_Error(YapicJson_Err_UnpairedHighSurrogate);
							return false;
						}
					} else if ((result & 0xFC00) == 0xDC00) {
						Decoder_Error(YapicJson_Err_UnpairedLowSurrogate);
						return false;
					}
				} break;

				default:
					if (*cursor == '\0') {
						Decoder_Error(YapicJson_Err_UnexpectedEnd);
						return false;
					} else {
						Decoder_Error(YapicJson_Err_InvalidEscape);
						return false;
					}
				break;
			}

			return true;
		}
};

template<typename CHIN, typename CHOUT, typename BUFF>
class BytesReader {
	public:
		static inline PyObject* Read(CHIN *&cursor, CHIN **cursorOut, const CHIN * const inputStart, const CHIN * const inputEnd, BUFF &buffer) {
			CHOUT maxchar = 127;
			CHOUT tmp = 0;

			while (cursor < inputEnd && MemoryBuffer_EnsureCapacity(buffer, 1)) {
				if (BR_IS_UTF8_ASCII(*cursor)) {
					IF_UNLIKELY (*cursor == '"') {
						goto success;
					} else IF_UNLIKELY (*cursor == '\\') {
						if (StringReader<CHIN, CHOUT, BUFF>::ReadEscapeSeq(cursor, inputStart, inputEnd, tmp)) {
							maxchar |= (*(buffer.cursor++) = tmp);
							cursor += 1;
						} else {
							return NULL;
						}
					} else {
						*(buffer.cursor++) = *(cursor++);
					}
				} else IF_LIKELY (ReadChar(cursor, inputEnd, tmp)) {
					maxchar |= (*(buffer.cursor++) = tmp);
				} else {
					return Decoder_Error(YapicJson_Err_UTF8Invalid);
				}
			}

			return Decoder_Error(YapicJson_Err_UnexpectedEnd);
			success:
				*cursorOut = ++cursor;
				return buffer.NewString(maxchar);
		}

		static inline bool ReadChar(CHIN *&cursor, const CHIN * const inputEnd, CHOUT &result) {
			if (*cursor < 0xC0) {
				return false;
			} else if (BR_IS_UTF8_LENGTH_2(*cursor)
						&& BR_IS_UTF8_CONT(*(cursor + 1))) {
				result = ((*cursor & UTF8_OCT2_MASK) << 6)
					| (*(cursor + 1) & UTF8_OCT_PART_MASK);

				cursor += 2;
			} else if (BR_IS_UTF8_LENGTH_3(*cursor)
						&& BR_IS_UTF8_CONT(*(cursor + 1))
						&& BR_IS_UTF8_CONT(*(cursor + 2))) {
				result = ((*cursor & UTF8_OCT3_MASK) << 12)
					| ((*(cursor + 1) & UTF8_OCT_PART_MASK) << 6)
					| (*(cursor + 2) & UTF8_OCT_PART_MASK);

				if (Py_UNICODE_IS_SURROGATE(result)) {
					return false;
				}

				cursor += 3;
			} else if (BR_IS_UTF8_LENGTH_4(*cursor)
						&& BR_IS_UTF8_CONT(*(cursor + 1))
						&& BR_IS_UTF8_CONT(*(cursor + 2))
						&& BR_IS_UTF8_CONT(*(cursor + 3))) {
				result = ((*cursor & UTF8_OCT4_MASK) << 18)
					| ((*(cursor + 1) & UTF8_OCT_PART_MASK) << 12)
					| ((*(cursor + 2) & UTF8_OCT_PART_MASK) << 6)
					| (*(cursor + 3) & UTF8_OCT_PART_MASK);

				if (result > 0x10FFFF) {
					return false;
				}

				cursor += 4;
			} else {
				return false;
			}

			return true;
		}
};


template<typename CHIN, typename CHOUT, typename BUFFER, typename READER>
class Decoder {
	public:
		CHIN* inputStart;
		CHIN* inputEnd;

		PyObject* objectHook;
		PyObject* parseFloat;
		bool parseDate;
		BUFFER strBuffer;

		inline Decoder(CHIN* data, size_t length)
			: inputStart(data), inputEnd(data + length) {
		}

		inline PyObject* Decode() {
			CHIN* end = NULL;
			PyObject* result = ReadValue(inputStart, &end);
			if (result != NULL) {
				Decoder_EatWhiteSpace(end);
				if (end != inputEnd) {
					Py_DECREF(result);
					PyErr_Format(Module::State()->DecodeError, YapicJson_Err_JunkTrailingData " at position: %ld.", end - inputStart);
					return NULL;
				}
			}
			return result;
		}

		Decoder_FN(ReadValue) {
			Decoder_EatWhiteSpace(cursor);
			switch (*cursor) {
				case '"':
					return ReadString(++cursor, cursorOut);
				break;

				case '{':
					return ReadDict(cursor, cursorOut);
				break;

				case '[':
					return ReadList(cursor, cursorOut);
				break;

				case 't':
					return ReadTrue(cursor, cursorOut);
				break;

				case 'f':
					return ReadFalse(cursor, cursorOut);
				break;

				case 'n':
					return ReadNull(cursor, cursorOut);
				break;
			}

			return ReadNumber(cursor, cursorOut);
		}

	private:
		char floatBuffer[YAPIC_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS + 10];

		Decoder_FN(ReadString) {
			PyObject* tmp = NULL;

			if (parseDate && __read_date(cursor, cursorOut, &tmp)) {
				// when parsed invalid date like string, eg: 8922-00-01
				// just continue string parsing
				if (tmp == NULL && PyErr_Occurred()) {
					PyErr_Clear();
				} else {
					return tmp;
				}
			}

			if (sizeof(CHIN) == 1) {
				if (__read_ascii(cursor, cursorOut, tmp)) {
					return tmp;
				} else {
					strBuffer.Reset();
					if (*cursorOut - cursor > 0 && !strBuffer.AppendSlice(cursor, *cursorOut - cursor)) {
						return NULL;
					}
					cursor = *cursorOut;
				}
			} else {
				strBuffer.Reset();
			}

			return READER::Read(cursor, cursorOut, inputStart, inputEnd, strBuffer);
		}

		inline bool __read_ascii(CHIN* cursor, CHIN **cursorOut, PyObject *&result) {
			CHIN *begin = cursor;
			while (str_state_table[*cursor] == STR_ASCII && cursor < inputEnd) {
				++cursor;
			}

			if (*cursor == '"') {
				Py_ssize_t size = cursor - begin;
				if ((result = PyUnicode_New(size, 127))) {
					assert(PyUnicode_KIND(result) == PyUnicode_1BYTE_KIND);
					CopyBytes(PyUnicode_1BYTE_DATA(result), begin, size);
					*cursorOut = cursor + 1;
					return true;
				} else {
					PyErr_Clear();
				}
			}

			*cursorOut = cursor;
			return false;
		}

		#define Decoder_IsNumber(ch) \
			((ch) >= '0' && (ch) <= '9')

		#define Decoder_Digit(ch) \
			((ch) - '0')

		struct DateParser {
			static inline bool ReadYear(CHIN* cursor, CHIN** cursorOut, int* year) {
				if (Decoder_IsNumber(cursor[0]) &&
					Decoder_IsNumber(cursor[1]) &&
					Decoder_IsNumber(cursor[2]) &&
					Decoder_IsNumber(cursor[3])) {
					*year = Decoder_Digit(cursor[0]) * 1000 +
							Decoder_Digit(cursor[1]) * 100 +
							Decoder_Digit(cursor[2]) * 10 +
							Decoder_Digit(cursor[3]);
					*cursorOut = cursor + 4;
					return true;
				} else {
					return false;
				}
			}

			static inline bool ReadTwoDigit(CHIN* cursor, CHIN** cursorOut, int* result) {
				if (Decoder_IsNumber(cursor[0]) &&
					Decoder_IsNumber(cursor[1])) {
					*result = Decoder_Digit(cursor[0]) * 10 + Decoder_Digit(cursor[1]);
					*cursorOut = cursor + 2;
					return true;
				} else {
					return false;
				}
			}

			static inline bool ReadDate(CHIN* cursor, CHIN** cursorOut, int* y, int* m, int* d) {
				return ReadYear(cursor, &cursor, y) && ReadCh<'-'>(cursor, &cursor) &&
					   ReadTwoDigit(cursor, &cursor, m) && ReadCh<'-'>(cursor, &cursor) &&
					   ReadTwoDigit(cursor, cursorOut, d);
			}

			static inline bool ReadTime(CHIN* cursor, CHIN** cursorOut, int* h, int* m, int* s, int* f) {
				if (ReadTwoDigit(cursor, &cursor, h) && ReadCh<':'>(cursor, &cursor) &&
					ReadTwoDigit(cursor, &cursor, m) && ReadCh<':'>(cursor, &cursor) &&
					ReadTwoDigit(cursor, &cursor, s)) {
					ReadFraction(cursor, &cursor, f);
					*cursorOut = cursor;
					return true;
				} else {
					return false;
				}
			}

			template<char ch>
			static inline bool ReadCh(CHIN* cursor, CHIN** cursorOut) {
				if (*cursor == ch) {
					*cursorOut = cursor + 1;
					return true;
				} else {
					return false;
				}
			}

			static inline bool ReadFraction(CHIN* cursor, CHIN** cursorOut, int* result) {
				if (*(cursor++) == '.') {
					CHIN* start = cursor;
					int factor = 100000;
					if (Decoder_IsNumber(*cursor)) {
						do {
							if (factor > 0) {
								*result = *result + (*(cursor) - '0') * factor;
								factor = factor / 10;
							}
							++cursor;
						} while (Decoder_IsNumber(*cursor));

						*cursorOut = cursor;
						return true;
					}
				}
				return false;
			}

			static inline bool ReadTZ(CHIN* cursor, CHIN** cursorOut, int* result) {
				if (*cursor == 'Z' || *cursor == 'z') {
					*result = 0;
					*cursorOut = cursor + 1;
					return true;
				} else {
					bool isNegative = false;
					if (*cursor == '-') {
						isNegative = true;
						cursor++;
					} else if (*cursor == '+') {
						cursor++;
					} else {
						return false;
					}

					int hour = 0;
					int minute = 0;

					if (ReadTwoDigit(cursor, &cursor, &hour)) {
						if (*cursor == ':') {
							cursor++;
						}
						if (ReadTwoDigit(cursor, &cursor, &minute)) {
							*result = hour * 3600 + minute * 60;
							if (isNegative) {
								*result = -*result;
							}
							*cursorOut = cursor;
							return true;
						}
					}
				}
				return false;
			}

			static inline PyObject* NewDate(const int& Y, const int& M, const int& D) {
				DateParser_CheckDate(Y, M, D);
				return PyDate_FromDate(Y, M, D);
			}

			static inline PyObject* NewTime(const int& h, const int& m, const int& s, const int& f) {
				DateParser_CheckTime(h, m, s, f);
				return PyTime_FromTime(h, m, s, f);
			}

			static inline PyObject* NewTime(const int& h, const int& m, const int& s, const int& f,
											const int& tz) {
				DateParser_CheckTime(h, m, s, f);
				PyObject* tzinfo = NewTZ(tz);
				if (tzinfo != NULL) {
					return PyDateTimeAPI->Time_FromTime(h, m, s, f, tzinfo, PyDateTimeAPI->TimeType);
				} else {
					return NULL;
				}
			}

			static inline PyObject* NewDateTime(const int& Y, const int& M, const int& D,
												const int& h, const int& m, const int& s, const int& f) {
				DateParser_CheckDate(Y, M, D);
				DateParser_CheckTime(h, m, s, f);
				return PyDateTime_FromDateAndTime(Y, M, D, h, m, s, f);
			}

			static inline PyObject* NewDateTime(const int& Y, const int& M, const int& D,
												const int& h, const int& m, const int& s, const int& f,
												const int& tz) {
				DateParser_CheckDate(Y, M, D);
				DateParser_CheckTime(h, m, s, f);
				PyObject* tzinfo = NewTZ(tz);
				if (tzinfo != NULL) {
					return PyDateTimeAPI->DateTime_FromDateAndTime(Y, M, D, h, m, s, f, tzinfo, PyDateTimeAPI->DateTimeType);
				} else {
					return NULL;
				}
			}

			static inline PyObject* NewTZ(const int& tz) {
				if (tz == 0) {
					return Module::State()->PyUTCTimezone;
				} else {
					PyObject* delta = PyDelta_FromDSU(0, tz, 0);
					if (delta == NULL) {
						return NULL;
					}
					PyObject* tzinfo = PyObject_CallFunctionObjArgs(Module::State()->PyTimezone, delta, NULL);
					Py_DECREF(delta);
					return tzinfo;
				}
			}
		};

		#define ReadDate_Return(TYPE, ...) \
			if (*cursor == '"') { \
				*cursorOut = cursor + 1; \
				*result = DateParser::New ## TYPE (__VA_ARGS__); \
				return true; \
			} else { \
				return false; \
			}

		/*
			date-fullyear   = 4DIGIT
			date-month      = 2DIGIT  ; 01-12
			date-mday       = 2DIGIT  ; 01-28, 01-29, 01-30, 01-31 based on
									  ; month/year
			time-hour       = 2DIGIT  ; 00-23
			time-minute     = 2DIGIT  ; 00-59
			time-second     = 2DIGIT  ; 00-58, 00-59, 00-60 based on leap second
									  ; rules
			time-secfrac    = "." 1*DIGIT
			time-numoffset  = ("+" / "-") time-hour ":" time-minute
			time-offset     = "Z" / time-numoffset

			partial-time    = time-hour ":" time-minute ":" time-second
							[time-secfrac]
			full-date       = date-fullyear "-" date-month "-" date-mday
			full-time       = partial-time time-offset

			date-time       = full-date "T" full-time
		*/
		inline bool __read_date(CHIN* cursor, CHIN** cursorOut, PyObject** result) {
			int Y=0, M=0, D=0,
				h=0, m=0, s=0, f=0,
				tz=0;

			if (DateParser::ReadDate(cursor, &cursor, &Y, &M, &D)) {
				if (*cursor == ' ' || *cursor == 't' || *cursor == 'T') {
					if (DateParser::ReadTime(++cursor, &cursor, &h, &m, &s, &f)) {
						if (DateParser::ReadTZ(cursor, &cursor, &tz)) {
							ReadDate_Return(DateTime, Y, M, D, h, m, s, f, tz);
						} else {
							ReadDate_Return(DateTime, Y, M, D, h, m, s, f);
						}
					}
				} else {
					ReadDate_Return(Date, Y, M, D);
				}
			} else if (DateParser::ReadTime(cursor, &cursor, &h, &m, &s, &f)) {
				if (DateParser::ReadTZ(cursor, &cursor, &tz)) {
					ReadDate_Return(Time, h, m, s, f, tz);
				} else {
					ReadDate_Return(Time, h, m, s, f);
				}
			}

			return false;
		}

		template<typename _Int>
		struct NegativeNumberTrait {
			typedef _Int Int;
			static constexpr bool IsNegative = true;
			static constexpr Int Limit = std::numeric_limits<Int>::min() / 10;
			static inline const Int Convert(const CHIN* ch) { return '0' - *ch; };
			static inline const bool IsOk(const Int& value) { return Limit <= value; }
			static inline const bool CheckOverflow(const Int& value) { return value <= 0; }
		};

		template<typename _Int>
		struct PositiveNumberTrait {
			typedef _Int Int;
			static constexpr bool IsNegative = false;
			static constexpr Int Limit = std::numeric_limits<Int>::max() / 10;
			static inline const Int Convert(const CHIN* ch) { return *ch - '0'; };
			static inline const bool IsOk(const Int& value) { return Limit >= value; }
			static inline const bool CheckOverflow(const Int& value) { return value >= 0; }
		};

		struct FFInternal {
			static constexpr bool IsInternal = true;
			static inline void ConsumeDecimalPoint(char* buff, char** buffOut) {}
			static inline void ConsumeFraction(char* buff, char** buffOut, const CHIN* ch, int* exponent) {
				*buff = *ch;
				*buffOut = buff + 1;
				--*exponent;
			}
			static inline void ExponentBegin(char* buff, char** buffOut, const char* buffEnd, const bool isNegative) {}
			static inline void ConsumeExponent(char* buff, char** buffOut, const CHIN* ch, const int& digit, int* exponent) {
				*exponent = *exponent * 10 + digit;
			}
		};

		struct FFExternal {
			static constexpr bool IsInternal = false;
			static inline void ConsumeDecimalPoint(char* buff, char** buffOut) {
				*buff = '.';
				*buffOut = buff + 1;
			}
			static inline void ConsumeFraction(char* buff, char** buffOut, const CHIN* ch, int* exponent) {
				*buff = *ch;
				*buffOut = buff + 1;
			}
			static inline void ExponentBegin(char* buff, char** buffOut, const char* buffEnd, const bool isNegative) {
				if (isNegative) {
					if (buff + 2 < buffEnd) {
						buff[0] = 'e';
						buff[1] = '-';
						*buffOut = buff + 2;
					}
				} else if (buff < buffEnd) {
					*buff = 'e';
					*buffOut = buff + 1;
				}
			}
			static inline void ConsumeExponent(char* buff, char** buffOut, const CHIN* ch, const int& digit, int* exponent) {
				*buff = *ch;
				*buffOut = buff + 1;
			}
		};

		Decoder_FN(ReadNumber) {
			if (*cursor == '-') {
				return parseFloat
					? __read_number<NegativeNumberTrait<long long>, FFExternal>(++cursor, cursorOut)
					: __read_number<NegativeNumberTrait<long long>, FFInternal>(++cursor, cursorOut);
			} else {
				return parseFloat
					? __read_number<PositiveNumberTrait<long long>, FFExternal>(cursor, cursorOut)
					: __read_number<PositiveNumberTrait<long long>, FFInternal>(cursor, cursorOut);
			}
		}

		template<typename Trait, typename FloatTrait>
		inline PyObject* __read_number(CHIN *cursor, CHIN **cursorOut) {
			typename Trait::Int intValue = 0;
			char* floatData = floatBuffer;
			char* floatDataEnd = floatBuffer + YAPIC_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS;
			int exponent = 0;

			if (Trait::IsNegative && !FloatTrait::IsInternal) {
				*(floatData++) = '-';
			}

			if (*cursor >= '1' && *cursor <= '9') {
				do {
					intValue = intValue * 10 + Trait::Convert(cursor);
					*(floatData++) = *(cursor++);
				} while (*cursor >= '0' && *cursor <= '9' && Trait::IsOk(intValue));

				if (*cursor == '.') {
					goto read_fraction;
				} else if (*cursor == 'e' || *cursor == 'E') {
					goto read_exponent;
				} else if (*cursor >= '0' && *cursor <= '9') {
					// int overflow, so we will handle as double
					do {
						*(floatData++) = *(cursor++);
					} while (*cursor >= '0' && *cursor <= '9' && floatData < floatDataEnd);
				} else if (Trait::CheckOverflow(intValue)) {
					*cursorOut = cursor;
					return PyLong_FromLongLong(intValue);
				}
			} else if (*cursor == '0') {
				++cursor;
				if (*cursor == '.') {
					*(floatData++) = '0';
					goto read_fraction;
				} else if (*cursor == 'e' || *cursor == 'E') {
					*(floatData++) = '0';
					goto read_exponent;
				} else {
					*cursorOut = cursor;
					return PyLong_FromLong(0);
				}
			} else if (__read_infinity(cursor, cursorOut)) {
				if (Trait::IsNegative) {
					return PyFloat_FromDouble(-Py_HUGE_VAL);
				} else {
					return PyFloat_FromDouble(Py_HUGE_VAL);
				}
			} else if (__read_nan(cursor, cursorOut)) {
				Py_RETURN_NAN;
			} else {
				if (cursor >= inputEnd) {
					Decoder_Error(YapicJson_Err_UnexpectedEnd);
				} else {
					return Decoder_ErrorFormat(YapicJson_Err_UnexpectedChar, *cursor);
				}
				return NULL;
			}



			if (*cursor == '.') {
		read_fraction:
				FloatTrait::ConsumeDecimalPoint(floatData, &floatData);
				++cursor;
				if (*cursor >= '0' && *cursor <= '9' && floatData < floatDataEnd) {
					do {
						FloatTrait::ConsumeFraction(floatData, &floatData, cursor++, &exponent);
					} while (*cursor >= '0' && *cursor <= '9' && floatData < floatDataEnd);
				} else {
					Decoder_Error(YapicJson_Err_UnexpectedCharInNumber);
					return NULL;
				}
			}

			if (*cursor == 'e' || *cursor == 'E') {
		read_exponent:
				int e = 0;

				++cursor;
				if (*cursor == '-') {
					if (!__read_exponent<NegativeNumberTrait<int>, FloatTrait>(
							++cursor, &cursor, &floatData, floatDataEnd, &e)) {
						return NULL;
					}
				} else {
					if (*cursor == '+') {
						++cursor;
					}
					if (!__read_exponent<PositiveNumberTrait<int>, FloatTrait>(
							cursor, &cursor, &floatData, floatDataEnd, &e)) {
						return NULL;
					}
				}

				exponent += e;
			}

			*cursorOut = cursor;
			if (FloatTrait::IsInternal) {
				if (Trait::IsNegative) {
					return PyFloat_FromDouble(
						-Strtod(Vector<const char>(floatBuffer, floatData - floatBuffer), exponent));
				} else {
					return PyFloat_FromDouble(
						Strtod(Vector<const char>(floatBuffer, floatData - floatBuffer), exponent));
				}
			} else {
				PyObject* str = PyUnicode_New(floatData - floatBuffer, 127);
				if (str == NULL) {
					return NULL;
				}
				memmove(PyUnicode_DATA(str), floatBuffer, floatData - floatBuffer);
				PyObject* res = PyObject_CallFunctionObjArgs(parseFloat, str, NULL);
				Py_DECREF(str);
				return res;
			}
		}

		template<typename Trait, typename FloatTrait>
		inline bool __read_exponent(CHIN* cursor, CHIN **cursorOut,
									char** floatData, const char* floatDataEnd,
									typename Trait::Int* exponent) {
			if (*cursor >= '0' && *cursor <= '9' && *floatData < floatDataEnd) {
				FloatTrait::ExponentBegin(*floatData, floatData, floatDataEnd, Trait::IsNegative);
				do {
					FloatTrait::ConsumeExponent(*floatData, floatData, cursor, Trait::Convert(cursor), exponent);
				} while (*(++cursor) >= '0' && *cursor <= '9' && *floatData < floatDataEnd);
				*cursorOut = cursor;
				return true;
			} else {
				Decoder_Error(YapicJson_Err_UnexpectedCharInNumber);
				return false;
			}
		}

		static inline bool __read_infinity(CHIN* in, CHIN** out) {
			if (in[0] == 'I' && in[1] == 'n' && in[2] == 'f' && in[3] == 'i' &&
				in[4] == 'n' && in[5] == 'i' && in[6] == 't' && in[7] == 'y') {
				*out = in + 8;
				return true;
			} else {
				return false;
			}
		}

		static inline bool __read_nan(CHIN* in, CHIN** out) {
			if (in[0] == 'N' && in[1] == 'a' && in[2] == 'N') {
				*out = in + 3;
				return true;
			} else {
				return false;
			}
		}

		Decoder_FN(ReadList) {
			PyObject* list = PyList_New(0);
			PyObject* item;
			if (list == NULL) {
				return NULL;
			}

			++cursor;
			Decoder_EatWhiteSpace(cursor);

			if (*cursor == ']') {
				*cursorOut = ++cursor;
				return list;
			}

			for (;;) {
				if ((item = ReadValue(cursor, &cursor))) {
					PyList_Append(list, item);
					Py_DECREF(item);
					Decoder_EatWhiteSpace(cursor);

					if (*cursor == ',') {
						++cursor;
						// Decoder_EatWhiteSpace(cursor);
					} else if (*cursor == ']') {
						*cursorOut = ++cursor;
						return list;
					} else {
						if (*cursor == '\0') {
							Decoder_Error(YapicJson_Err_UnexpectedEnd);
						} else {
							Decoder_Error(YapicJson_Err_UnexpectedCharInList);
						}
						break;
					}
				} else {
					break;
				}
			}

			Py_DECREF(list);
			return NULL;
		}

		#define Decoder_DictError(expected) \
			if (*cursor == '\0') { \
				Decoder_Error(YapicJson_Err_UnexpectedEnd); \
			} else { \
				Decoder_Error(YapicJson_Err_UnexpectedCharInDict expected); \
			}

		Decoder_FN(ReadDict) {
			PyObject* dict = PyDict_New();
			PyObject* key = NULL;
			PyObject* value = NULL;

			if (dict == NULL) {
				return NULL;
			}

			++cursor;
			Decoder_EatWhiteSpace(cursor);

			if (*cursor == '}') {
				*cursorOut = ++cursor;
				return dict;
			}

			for (;;) {
				if (*cursor == '"') {
					if ((key = ReadString(++cursor, &cursor))) {
						Decoder_EatWhiteSpace(cursor);
						if (*(cursor++) == ':') {
							if ((value = ReadValue(cursor, &cursor))) {
								if (PyDict_SetItem(dict, key, value) == 0) {
									Py_DECREF(key);
									key = NULL;
									Py_DECREF(value);
									value = NULL;
									Decoder_EatWhiteSpace(cursor);
									if (*cursor == ',') {
										++cursor;
										Decoder_EatWhiteSpace(cursor);
									} else if (*cursor == '}') {
										*cursorOut = ++cursor;

										if (objectHook) {
											PyObject* result = PyObject_CallFunctionObjArgs(objectHook, dict, NULL);
											Py_DECREF(dict);
											return result;
										}

										return dict;
									} else {
										Decoder_DictError("',', '}'");
										break;
									}
								} else {
									break;
								}
							} else {
								break;
							}
						} else {
							--cursor;
							Decoder_DictError("':'");
							break;
						}
					} else {
						break;
					}
				} else {
					Decoder_DictError("'\"'");
					break;
				}
			}

			if (key != NULL) {
				Py_DECREF(key);
			}

			if (value != NULL) {
				Py_DECREF(value);
			}
			Py_DECREF(dict);
			return NULL;
		}

		Decoder_FN(ReadTrue) {
			if (cursor[1] == 'r' && cursor[2] == 'u' && cursor[3] == 'e') {
				*cursorOut = cursor + 4;
				Py_RETURN_TRUE;
			}
			Decoder_Error(YapicJson_Err_UnexpectedCharInTrue);
			return NULL;
		}

		Decoder_FN(ReadFalse) {
			if (cursor[1] == 'a' && cursor[2] == 'l' && cursor[3] == 's' && cursor[4] == 'e') {
				*cursorOut = cursor + 5;
				Py_RETURN_FALSE;
			}
			Decoder_Error(YapicJson_Err_UnexpectedCharInFalse);
			return NULL;
		}

		Decoder_FN(ReadNull) {
			if (cursor[1] == 'u' && cursor[2] == 'l' && cursor[3] == 'l') {
				*cursorOut = cursor + 4;
				Py_RETURN_NONE;
			}
			Decoder_Error(YapicJson_Err_UnexpectedCharInNull);
			return NULL;
		}
};


template<typename CHIN, typename CHOUT>
using StrDecoder = Decoder<CHIN, CHOUT, ChunkBuffer, StringReader<CHIN, CHOUT, ChunkBuffer>>;

template<typename CHIN, typename CHOUT, Py_ssize_t BSIZE>
using BytesDecoder = Decoder<CHIN, CHOUT, MemoryBuffer<CHOUT, BSIZE>, BytesReader<CHIN, CHOUT, MemoryBuffer<CHOUT, BSIZE>>>;


} /* end namespace Json */
} /* end namespace Yapic */


#endif /* QDD73D15_6133_C640_1481_736A723E63B6 */
