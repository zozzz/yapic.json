#ifndef QDD73D15_6133_C640_1481_736A723E63B6
#define QDD73D15_6133_C640_1481_736A723E63B6

#include <limits>

#include "../libs/double-conversion/double-conversion/double-conversion.h"
#include "../libs/double-conversion/double-conversion/strtod.h"
#include "config.h"
#include "chunk-buffer.h"


#define Decoder_Error(msg) \
	PyErr_Format(DecodeError, msg " at position: %ld.", cursor - inputStart)

#define Decoder_ErrorFormat(msg, ...) \
	PyErr_Format(DecodeError, msg " at position: %ld.", __VA_ARGS__, cursor - inputStart); \
	return NULL

#define Decoder_FN(name) \
	inline PyObject* name(CHIN* cursor, CHIN** cursorOut)

#define Decoder_IsWhiteSpace(ch) \
	((ch) == ' ' || ((ch) <= '\n' && (ch) >= '\t') || (ch) == '\r')

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


namespace ZiboJson {
using namespace double_conversion;


template<typename CHIN, typename BUFF>
class Decoder {
	public:
		typedef typename BUFF::Char CHOUT;

		CHIN* inputStart;
		CHIN* inputEnd;

		PyObject* objectHook;
		PyObject* parseFloat;
		bool parseDate;

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
					PyErr_Format(DecodeError, ZiboJson_Err_JunkTrailingData " at position: %ld.", end - inputStart);
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
		ChunkBuffer buffer;
		char floatBuffer[ZIBO_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS + 10];

		#define ReadUnicodeEscapePart(output) \
			++cursor; \
			if (*cursor >= '0' && *cursor <= '9')  { \
				output = (output << 4) + (*cursor - '0'); \
			} else if (*cursor >= 'a' && *cursor <= 'f') { \
				output = (output << 4) + (*cursor - 'a' + 10); \
			} else if (*cursor >= 'A' && *cursor <= 'F') { \
				output = (output << 4) + (*cursor - 'A' + 10); \
			} else { \
				if (*cursor == '\0') { Decoder_Error(ZiboJson_Err_UnexpectedEnd); } \
				else { Decoder_Error(ZiboJson_Err_UnexpectedCharInUnicodeEscape); } \
				return NULL; \
			}

		#define ReadUnicodeEscape(output) \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output); \
				ReadUnicodeEscapePart(output);


		Decoder_FN(ReadString) {
			if (parseDate) {
				PyObject* dt = NULL;
				if (__read_date(cursor, cursorOut, &dt)) {
					return dt;
				}
			}

			buffer.Reset();
			register CHOUT maxchar = 127;

			while (cursor < inputEnd) {
				// printf("cursor = %ld end = %ld L = %ld\n", cursor, end, end - cursor);

				if (*cursor == '"') {
					goto success;
				} else if (*cursor == '\\') {
					CHOUT ucs_pt1 = 0;
					switch (*(++cursor)) {
						case 'b': ucs_pt1 = '\b'; break;
						case 'f': ucs_pt1 = '\f'; break;
						case 'n': ucs_pt1 = '\n'; break;
						case 'r': ucs_pt1 = '\r'; break;
						case 't': ucs_pt1 = '\t'; break;
						case '"': ucs_pt1 = '"'; break;
						case '\\': ucs_pt1 = '\\'; break;
						case '/': ucs_pt1 = '/'; break;
						case 'u': {
							ReadUnicodeEscape(ucs_pt1);

							if ((ucs_pt1 & 0xFC00) == 0xD800) {
								if (*(++cursor) == '\\' && *(++cursor) == 'u' ) {
									CHOUT ucs_pt2 = 0;
									ReadUnicodeEscape(ucs_pt2);
									if ((ucs_pt2 & 0xFC00) == 0xDC00) {
										ucs_pt1 = 0x10000 + (((ucs_pt1 - 0xD800) << 10) | (ucs_pt2 - 0xDC00));
									} else {
										Decoder_Error(ZiboJson_Err_UnpairedHighSurrogate);
										return NULL;
									}
								} else {
									Decoder_Error(ZiboJson_Err_UnpairedHighSurrogate);
									return NULL;
								}
							} else if ((ucs_pt1 & 0xFC00) == 0xDC00) {
								Decoder_Error(ZiboJson_Err_UnpairedLowSurrogate);
								return NULL;
							}

							maxchar |= ucs_pt1;
						} break;

						default:
							if (*cursor == '\0') {
								Decoder_Error(ZiboJson_Err_UnexpectedEnd);
							} else {
								Decoder_Error(ZiboJson_Err_InvalidEscape);
							}
							return NULL;
						break;
					}

					if (buffer.AppendChar(ucs_pt1)) {
						++cursor;
					} else {
						return NULL;
					}
				} else {
					buffer.StartSlice(cursor);
					do {
						maxchar |= *cursor;
					} while (++cursor < inputEnd && *cursor != '\\' && *cursor != '"');

					if (!buffer.CloseSlice(cursor)) {
						return NULL;
					}
				}
			}

			Decoder_Error(ZiboJson_Err_UnexpectedEnd);
			return NULL;

		success:
			*cursorOut = ++cursor;
			return buffer.NewString(maxchar);
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
					if (Decoder_IsNumber(*cursor)) {
						do {
							*result = *result * 10 + (*(cursor++) - '0');
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
					return PyUTCTimezone;
				} else {
					PyObject* delta = PyDelta_FromDSU(0, tz, 0);
					if (delta == NULL) {
						return NULL;
					}
					PyObject* tzinfo = PyObject_CallFunctionObjArgs(PyTimezone, delta, NULL);
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
			} else {
				return false;
			}
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
			register typename Trait::Int intValue = 0;
			char* floatData = floatBuffer;
			char* floatDataEnd = floatBuffer + ZIBO_JSON_DOUBLE_MAX_SIGNIFICANT_DIGITS;
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
					return PyLong_FromLong(intValue);
				}
			} else if (*cursor == '0') {
				++cursor;
				if (*cursor == '.') {
					goto read_fraction;
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
					Decoder_Error(ZiboJson_Err_UnexpectedEnd);
				} else {
					Decoder_ErrorFormat(ZiboJson_Err_UnexpectedChar, *cursor);
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
					Decoder_Error(ZiboJson_Err_UnexpectedCharInNumber);
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
				Decoder_Error(ZiboJson_Err_UnexpectedCharInNumber);
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
			register PyObject* list = PyList_New(0);
			register PyObject* item;
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
							Decoder_Error(ZiboJson_Err_UnexpectedEnd);
						} else {
							Decoder_Error(ZiboJson_Err_UnexpectedCharInList);
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
				Decoder_Error(ZiboJson_Err_UnexpectedEnd); \
			} else { \
				Decoder_Error(ZiboJson_Err_UnexpectedCharInDict expected); \
			}

		Decoder_FN(ReadDict) {
			register PyObject* dict = PyDict_New();
			register PyObject* key = NULL;
			register PyObject* value = NULL;

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
									Py_DECREF(value);
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
										// decrefed above
										key = NULL;
										value = NULL;
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
					Decoder_DictError("\"");
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
			Decoder_Error(ZiboJson_Err_UnexpectedCharInTrue);
			return NULL;
		}

		Decoder_FN(ReadFalse) {
			if (cursor[1] == 'a' && cursor[2] == 'l' && cursor[3] == 's' && cursor[4] == 'e') {
				*cursorOut = cursor + 5;
				Py_RETURN_FALSE;
			}
			Decoder_Error(ZiboJson_Err_UnexpectedCharInFalse);
			return NULL;
		}

		Decoder_FN(ReadNull) {
			if (cursor[1] == 'u' && cursor[2] == 'l' && cursor[3] == 'l') {
				*cursorOut = cursor + 4;
				Py_RETURN_NONE;
			}
			Decoder_Error(ZiboJson_Err_UnexpectedCharInNull);
			return NULL;
		}
};

} /* end namespace ZiboJson */


#endif /* QDD73D15_6133_C640_1481_736A723E63B6 */
