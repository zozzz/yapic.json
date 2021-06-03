#ifndef DEFBD7C4_2133_C63D_128D_F5D3D59111EF
#define DEFBD7C4_2133_C63D_128D_F5D3D59111EF

#include <limits>
#include <yapic/pyptr.hpp>

#include "../libs/double-conversion/double-conversion/double-conversion.h"
#include "config.h"
#include "buffer.h"
#include "json.h"


namespace Yapic { namespace Json {
using namespace double_conversion;

#define Encoder_RETURN_TRUE return true

#define Encoder_RETURN_FALSE return false

#define Encoder_FN(name) inline bool name(PyObject* obj)

#define Encoder_AppendFast(chr) \
	( (assert(buffer.end - buffer.cursor >= 1)), (*(buffer.cursor++) = (chr)))

#define Encoder_EXTRA_CAPACITY 10

#define Encoder_EnsureCapacity(required) \
	if ((required) > buffer.end - buffer.cursor && buffer.EnsureCapacity(required) == false) { \
		Encoder_RETURN_FALSE; \
	}

#define Encoder_EnterRecursive() \
	if (++recursionDepth > maxRecursionDepth) { \
		Encoder_RETURN_FALSE; \
	}

#define Encoder_LeaveRecursive() \
	((assert(recursionDepth < maxRecursionDepth)), --recursionDepth)

#define Encoder_RecursionError(msg, ...) \
	PyErr_Format(Module::State()->EncodeError, YapicJson_Err_MaxRecursion msg, __VA_ARGS__)

#define Encoder_RecursionOccured() \
	(recursionDepth > maxRecursionDepth && !PyErr_Occurred())

#define Encoder_HandleRecursion(msg, ...) \
	if (Encoder_RecursionOccured()) { \
		Encoder_RecursionError(msg, __VA_ARGS__); \
		Encoder_RETURN_FALSE;

// static const char* __hex_chars = "0123456789abcdef";
// #define HEX_CHAR(idx) (__hex_chars[(idx)])

#define HEX_CHAR(idx) ("0123456789abcdef"[(idx)])

#define Encoder_UTF8_2BYTE_START 0xC0
#define Encoder_UTF8_3BYTE_START 0xE0
#define Encoder_UTF8_4BYTE_START 0xF0
#define Encoder_UTF8_CONTINUATION 0x80


template<typename BUFF, bool EnsureAscii>
class Encoder {
	public:
		typedef typename BUFF::Char CHOUT;
		BUFF buffer;

		PyObject* defaultFn;
		PyObject* toJsonMethodName;
		int maxRecursionDepth;
		int recursionDepth;
		bool encodeDatetime;

		inline explicit Encoder()
			: recursionDepth(0) {
		}

		inline bool Encode(PyObject* obj) {
			assert(obj != NULL);

			if (PyUnicode_CheckExact(obj)) {
				Encoder_AppendFast('"');
				IF_LIKELY (EncodeString(obj)) {
					Encoder_AppendFast('"');
					Encoder_RETURN_TRUE;
				} else {
					Encoder_RETURN_FALSE;
				}
			} else if (PyDict_CheckExact(obj)) {
				return EncodeDict(obj);
			} else if (PyList_CheckExact(obj)) {
				return EncodeList(obj);
			} else if (PyTuple_CheckExact(obj)) {
				return EncodeTuple(obj);
			} else if (PyDateTime_Check(obj)) {
				return EncodeDateTime(obj);
			} else if (PyDate_Check(obj)) {
				return EncodeDate(obj);
			} else if (PyTime_Check(obj)) {
				return EncodeTime(obj);
			} else if (PyFloat_CheckExact(obj)) {
				return EncodeFloat(obj);
			} else if (obj == Py_True) {
				Encoder_EnsureCapacity(4 + Encoder_EXTRA_CAPACITY);
				Encoder_AppendFast('t');
				Encoder_AppendFast('r');
				Encoder_AppendFast('u');
				Encoder_AppendFast('e');
				Encoder_RETURN_TRUE;
			} else if (obj == Py_False) {
				Encoder_EnsureCapacity(5 + Encoder_EXTRA_CAPACITY);
				Encoder_AppendFast('f');
				Encoder_AppendFast('a');
				Encoder_AppendFast('l');
				Encoder_AppendFast('s');
				Encoder_AppendFast('e');
				Encoder_RETURN_TRUE;
			} else if (obj == Py_None) {
				Encoder_EnsureCapacity(4 + Encoder_EXTRA_CAPACITY);
				Encoder_AppendFast('n');
				Encoder_AppendFast('u');
				Encoder_AppendFast('l');
				Encoder_AppendFast('l');
				Encoder_RETURN_TRUE;
			} else if (PyLong_Check(obj)) {
				return EncodeLong(obj);
			} else if (PyAnySet_Check(obj)) {
				return EncodeIterable(obj);
			} else if (PyObject_HasAttr(obj, toJsonMethodName)) {
				return EncodeWithJsonMethod<false>(obj);
			} else if (PyObject_IsInstance(obj, Module::State()->ItemsView)) {
				return EncodeItemsView(obj);
			} else if (Module::State()->Enum.Check(obj)) {
				return EncodeEnum<false>(obj);
			} else if (Module::State()->UUID.Check(obj)) {
				Encoder_EnsureCapacity(Encoder_EXTRA_CAPACITY);
				Encoder_AppendFast('"');
				IF_LIKELY (EncodeUUID(obj)) {
					Encoder_AppendFast('"');
					Encoder_RETURN_TRUE;
				} else {
					Encoder_RETURN_FALSE;
				}
			} else if (PyIter_Check(obj)) {
				return EncodeIterable(obj);
			} else if (PyCallable_Check(defaultFn)) {
				return EncodeWithDefault<false>(obj);
			} else if (Module::State()->Decimal.CheckExact(obj)) {
				return EncodeDecimal(obj);
			}

			PyErr_Format(Module::State()->EncodeError, YapicJson_Err_NotSerializable, obj);
			Encoder_RETURN_FALSE;
		}

	private:
		Encoder_FN(__encode_dict_key) {
			if (PyUnicode_CheckExact(obj)) {
				return EncodeString(obj);
			} else if (PyLong_CheckExact(obj)) {
				return EncodeLong(obj);
			} else if (PyFloat_CheckExact(obj)) {
				return EncodeFloat(obj);
			} else if (obj == Py_True) {
				Encoder_EnsureCapacity(4 + Encoder_EXTRA_CAPACITY);
				Encoder_AppendFast('t');
				Encoder_AppendFast('r');
				Encoder_AppendFast('u');
				Encoder_AppendFast('e');
				Encoder_RETURN_TRUE;
			} else if (obj == Py_False) {
				Encoder_EnsureCapacity(5 + Encoder_EXTRA_CAPACITY);
				Encoder_AppendFast('f');
				Encoder_AppendFast('a');
				Encoder_AppendFast('l');
				Encoder_AppendFast('s');
				Encoder_AppendFast('e');
				Encoder_RETURN_TRUE;
			} else if (obj == Py_None) {
				Encoder_EnsureCapacity(4 + Encoder_EXTRA_CAPACITY);
				Encoder_AppendFast('n');
				Encoder_AppendFast('u');
				Encoder_AppendFast('l');
				Encoder_AppendFast('l');
				Encoder_RETURN_TRUE;
			} else if (PyObject_HasAttr(obj, toJsonMethodName)) {
				return EncodeWithJsonMethod<true>(obj);
			} else if (PyCallable_Check(defaultFn)) {
				return EncodeWithDefault<true>(obj);
			} else if (Module::State()->Decimal.CheckExact(obj)) {
				return EncodeDecimal(obj);
			} else if (Module::State()->Enum.Check(obj)) {
				return EncodeEnum<true>(obj);
			} else if (Module::State()->UUID.Check(obj)) {
				return EncodeUUID(obj);
			}

			PyErr_Format(Module::State()->EncodeError, YapicJson_Err_InvalidDictKey, obj, toJsonMethodName);
			Encoder_RETURN_FALSE;
		}

		Encoder_FN(EncodeString) {
			Py_ssize_t length = PyUnicode_GET_LENGTH(obj);
			void* data = PyUnicode_1BYTE_DATA(obj);

			switch (PyUnicode_KIND(obj)) {
				case PyUnicode_1BYTE_KIND:
					Encoder_EnsureCapacity(length * 6 + Encoder_EXTRA_CAPACITY);
					__encode_string(reinterpret_cast<Py_UCS1*>(data), reinterpret_cast<Py_UCS1*>(data) + length);
				break;

				case PyUnicode_2BYTE_KIND:
					Encoder_EnsureCapacity(length * 6 + Encoder_EXTRA_CAPACITY);
					__encode_string(reinterpret_cast<Py_UCS2*>(data), reinterpret_cast<Py_UCS2*>(data) + length);
				break;

				case PyUnicode_4BYTE_KIND:
					if (sizeof(CHOUT) == 1) {
						if (EnsureAscii) {
							Encoder_EnsureCapacity(length * 12 + Encoder_EXTRA_CAPACITY);
						} else {
							Encoder_EnsureCapacity(length * 6 + Encoder_EXTRA_CAPACITY);
						}
					} else {
						Encoder_EnsureCapacity(length * 6 + Encoder_EXTRA_CAPACITY);
					}
					__encode_string(reinterpret_cast<Py_UCS4*>(data), reinterpret_cast<Py_UCS4*>(data) + length);
				break;
			}

			Encoder_RETURN_TRUE;
		}

		#define StringEncoder_AppendChar(ch) \
				((assert(buffer.end - out >= 1)), (*(out++) = (ch)))

		template<typename CHIN>
		inline void __encode_string(const CHIN* input, const CHIN* end) {
			CHOUT* out = buffer.cursor;
			CHOUT maxchar = buffer.maxchar;
			CHIN ch;

			for (;;) {
				if ((ch = *input) < (EnsureAscii ? 127 : 128)) { // ASCII -> ASCII | UNICODE
					IF_LIKELY (ch > 31 && ch != '\\' && ch != '"') {
						StringEncoder_AppendChar(ch);
					} else if (input >= end) {
						break;
					} else {
						__encode_escapes(out, ch);
					}
				} else if (EnsureAscii) {
					StringEncoder_AppendChar('\\');
					StringEncoder_AppendChar('u');
					if (sizeof(CHIN) == 1) {
						StringEncoder_AppendChar('0');
						StringEncoder_AppendChar('0');
						StringEncoder_AppendChar(HEX_CHAR((ch & 0xF0) >> 4));
						StringEncoder_AppendChar(HEX_CHAR((ch & 0x0F)));
					} else if (sizeof(CHIN) == 2) {
						StringEncoder_AppendChar(HEX_CHAR( (ch >> 12) ));
						StringEncoder_AppendChar(HEX_CHAR( (ch >> 8) & 0xF ));
						StringEncoder_AppendChar(HEX_CHAR( (ch >> 4) & 0xF ));
						StringEncoder_AppendChar(HEX_CHAR( ch & 0xF ));
					} else if (sizeof(CHIN) == 4) {
						if (ch > 0xFFFF) {
							CHIN high = 0xD800 - (0x10000 >> 10) + (ch >> 10);
							StringEncoder_AppendChar('d');
							StringEncoder_AppendChar(HEX_CHAR( (high >> 8) & 0xF ));
							StringEncoder_AppendChar(HEX_CHAR( (high >> 4) & 0xF ));
							StringEncoder_AppendChar(HEX_CHAR( high & 0xF));

							StringEncoder_AppendChar('\\');
							StringEncoder_AppendChar('u');
							ch = 0xDC00 + (ch & 0x3FF);
						}

						StringEncoder_AppendChar(HEX_CHAR( (ch >> 12) ));
						StringEncoder_AppendChar(HEX_CHAR( (ch >> 8) & 0xF ));
						StringEncoder_AppendChar(HEX_CHAR( (ch >> 4) & 0xF ));
						StringEncoder_AppendChar(HEX_CHAR( ch & 0xF ));
					}
				} else if (sizeof(CHOUT) == 1) {
					if (ch < 0x0800 ) { // 2 byte
						StringEncoder_AppendChar( Encoder_UTF8_2BYTE_START | (ch >> 6) );
						StringEncoder_AppendChar( Encoder_UTF8_CONTINUATION | (ch & 0x3F) );
					} else if (ch < 0x10000) { // 3 byte
						StringEncoder_AppendChar( Encoder_UTF8_3BYTE_START | (ch >> 12) );
						StringEncoder_AppendChar( Encoder_UTF8_CONTINUATION | (ch >> 6 & 0x3F) );
						StringEncoder_AppendChar( Encoder_UTF8_CONTINUATION | (ch & 0x3F) );
					} else if (sizeof(CHIN) == 4) { // 4 byte
						StringEncoder_AppendChar( Encoder_UTF8_4BYTE_START | (ch >> 18) );
						StringEncoder_AppendChar( Encoder_UTF8_CONTINUATION | (ch >> 12 & 0x3F) );
						StringEncoder_AppendChar( Encoder_UTF8_CONTINUATION | (ch >> 6 & 0x3F) );
						StringEncoder_AppendChar( Encoder_UTF8_CONTINUATION | (ch & 0x3F) );
					}
				} else {
					assert(sizeof(CHIN) <= sizeof(CHOUT));
					maxchar |= ch;
					StringEncoder_AppendChar(ch);
				}
				input += 1;
			}

			buffer.cursor = out;
			buffer.maxchar = maxchar;
		}

		template<typename CHIN>
		bool __encode_escapes(CHOUT *&out, CHIN &ch) {
			StringEncoder_AppendChar('\\');
			switch (ch) {
				case '\r': StringEncoder_AppendChar('r'); break;
				case '\n': StringEncoder_AppendChar('n'); break;
				case '\t': StringEncoder_AppendChar('t'); break;
				case '\b': StringEncoder_AppendChar('b'); break;
				case '\f': StringEncoder_AppendChar('f'); break;
				case '\\': StringEncoder_AppendChar('\\'); break;
				case '"': StringEncoder_AppendChar('"'); break;
				default:
					StringEncoder_AppendChar('u');
					StringEncoder_AppendChar('0');
					StringEncoder_AppendChar('0');
					StringEncoder_AppendChar(HEX_CHAR((ch & 0xF0) >> 4));
					StringEncoder_AppendChar(HEX_CHAR((ch & 0x0F)));
				break;
			}
			return false;
		}

		Encoder_FN(EncodeLong) {
			int is_overflow = 0;
			long long value = PyLong_AsLongLongAndOverflow(obj, &is_overflow);

			if (is_overflow != 0) {
				PyErr_SetString(Module::State()->EncodeError, YapicJson_Err_IntOverflow);
				return false;
			}

			Encoder_EnsureCapacity(LLONG_MAX_LENGTH_IN_CHR + Encoder_EXTRA_CAPACITY);
			unsigned long long abs_value = value;

			if (value < 0) {
				abs_value = -value;
				Encoder_AppendFast('-');
			}

			CHOUT *end_position = buffer.cursor + LLONG_MAX_LENGTH_IN_CHR;
			CHOUT *saved_end_position = end_position;

			do {
				*(--end_position) = (48 + (abs_value % 10));
			} while ((abs_value /= 10) > 0);

			#pragma warning(suppress: 4244)
			abs_value = saved_end_position - end_position;

			memmove(buffer.cursor, end_position, sizeof(CHOUT) * abs_value);
			buffer.cursor += abs_value;

			Encoder_RETURN_TRUE;
		}

		Encoder_FN(EncodeFloat) {
			Encoder_EnsureCapacity(DOUBLE_MAX_LENGTH_IN_CHR + Encoder_EXTRA_CAPACITY);

			if (sizeof(CHOUT) == 1) {
				StringBuilder builder((char*) buffer.cursor, DOUBLE_MAX_LENGTH_IN_CHR);

				DoubleToStringConverter::EcmaScriptConverter().ToShortest(
					PyFloat_AS_DOUBLE(obj),
					&builder
				);

				buffer.cursor += builder.position();
			} else {
				char tmp[DOUBLE_MAX_LENGTH_IN_CHR];
				StringBuilder builder(tmp, DOUBLE_MAX_LENGTH_IN_CHR);

				DoubleToStringConverter::EcmaScriptConverter().ToShortest(
					PyFloat_AS_DOUBLE(obj),
					&builder
				);

				int size = builder.position();
				if (size) {
					buffer.cursor += size;
					CHOUT* cursor = buffer.cursor - 1;
					do {
						*(cursor--) = tmp[--size];
					} while(size);
				}
			}
			Encoder_RETURN_TRUE;
		}

		Encoder_FN(EncodeDecimal) {
			PyObject* str = PyObject_Str(obj);
			if (str == NULL) {
				Encoder_RETURN_FALSE;
			}
			bool res = EncodeString(str);
			Py_DECREF(str);
			return res;
		}

		#define EncodeDT_AppendInt2(value) \
			Encoder_AppendFast('0' + (value / 10)); \
			Encoder_AppendFast('0' + (value % 10));

		#define EncodeDT_MILIS_MAX_LENGTH 6
		#define EncodeDT_AppendMilis(value, ms_size) \
			{ \
				ms_size = value; \
				CHOUT *end_position = buffer.cursor + EncodeDT_MILIS_MAX_LENGTH; \
				CHOUT digit; \
				do { \
					digit = (48 + (ms_size % 10)); \
					*(--end_position) = digit; \
					ms_size /= 10; \
				} while (end_position > buffer.cursor); \
				ms_size = EncodeDT_MILIS_MAX_LENGTH; \
				memmove(buffer.cursor, end_position, sizeof(CHOUT) * ms_size); \
				buffer.cursor += ms_size; \
			}

		#define EncodeDT_AppendInt4(value) \
			Encoder_AppendFast('0' + (value / 1000)); \
			Encoder_AppendFast('0' + ((value / 100) % 10)); \
			Encoder_AppendFast('0' + ((value / 10) % 10)); \
			Encoder_AppendFast('0' + (value % 10));

		Encoder_FN(EncodeDate) {
			Encoder_EnsureCapacity(12 + Encoder_EXTRA_CAPACITY);

			int y = PyDateTime_GET_YEAR(obj);
			int m = PyDateTime_GET_MONTH(obj);
			int d = PyDateTime_GET_DAY(obj);

			Encoder_AppendFast('"');
			EncodeDT_AppendInt4(y);
			Encoder_AppendFast('-');
			EncodeDT_AppendInt2(m);
			Encoder_AppendFast('-');
			EncodeDT_AppendInt2(d);
			Encoder_AppendFast('"');

			Encoder_RETURN_TRUE;
		}

		// TODO: maybe tz info
		// "22:54:12.123456+01:00"
		Encoder_FN(EncodeTime) {
			Encoder_EnsureCapacity(23 + Encoder_EXTRA_CAPACITY);

			int h = PyDateTime_TIME_GET_HOUR(obj);
			int m = PyDateTime_TIME_GET_MINUTE(obj);
			int s = PyDateTime_TIME_GET_SECOND(obj);
			int ms = PyDateTime_TIME_GET_MICROSECOND(obj);
			size_t ms_size = 0;

			Encoder_AppendFast('"');
			EncodeDT_AppendInt2(h);
			Encoder_AppendFast(':');
			EncodeDT_AppendInt2(m);
			if (s > 0 || ms > 0) {
				Encoder_AppendFast(':');
				EncodeDT_AppendInt2(s);
			}
			if (ms > 0) {
				Encoder_AppendFast('.');
				EncodeDT_AppendMilis(ms, ms_size);
			}
			Encoder_AppendFast('"');

			Encoder_RETURN_TRUE;
		}

		// "2017-04-02T22:54:12.123456+01:00"
		Encoder_FN(EncodeDateTime) {
			Encoder_EnsureCapacity(34 + Encoder_EXTRA_CAPACITY);

			int dy = PyDateTime_GET_YEAR(obj);
			int dm = PyDateTime_GET_MONTH(obj);
			int dd = PyDateTime_GET_DAY(obj);
			int th = PyDateTime_DATE_GET_HOUR(obj);
			int tm = PyDateTime_DATE_GET_MINUTE(obj);
			int ts = PyDateTime_DATE_GET_SECOND(obj);
			int tms = PyDateTime_DATE_GET_MICROSECOND(obj);
			size_t ms_size = 0;

			Encoder_AppendFast('"');
			EncodeDT_AppendInt4(dy);
			Encoder_AppendFast('-');
			EncodeDT_AppendInt2(dm);
			Encoder_AppendFast('-');
			EncodeDT_AppendInt2(dd);
			Encoder_AppendFast(' ');
			EncodeDT_AppendInt2(th);
			Encoder_AppendFast(':');
			EncodeDT_AppendInt2(tm);
			Encoder_AppendFast(':');
			EncodeDT_AppendInt2(ts);
			if (tms > 0) {
				Encoder_AppendFast('.');
				EncodeDT_AppendMilis(tms, ms_size);
			}

			PyObject* tzinfo = PyObject_GetAttr(obj, Module::State()->STR_TZINFO);
			if (tzinfo == NULL) {
				Encoder_RETURN_FALSE;
			} else if (tzinfo != Py_None) {
				PyObject *delta = PyObject_CallMethodObjArgs(tzinfo, Module::State()->STR_UTCOFFSET, obj, NULL);
				Py_DECREF(tzinfo);

				if (delta == NULL) {
					Encoder_RETURN_FALSE;
				} else if (PyDelta_Check(delta)) {
					int utcoffset = ((PyDateTime_Delta*)delta)->seconds + ((PyDateTime_Delta*)delta)->days * 86400;
					Py_DECREF(delta);

					if (ms_size > 0) {
						*(buffer.cursor - 10 - ms_size) = 'T';
					} else {
						*(buffer.cursor - 9) = 'T';
					}

					if (utcoffset == 0) {
						Encoder_AppendFast('Z');
					} else {
						if (utcoffset < 0) {
							utcoffset = -utcoffset;
							Encoder_AppendFast('-');
						} else {
							Encoder_AppendFast('+');
						}
						int tzm = utcoffset / 60;
						int tzh = (tzm / 60) % 24;
						tzm %= 60;
						EncodeDT_AppendInt2(tzh);
						Encoder_AppendFast(':');
						EncodeDT_AppendInt2(tzm);
					}
				} else {
					PyErr_Format(PyExc_TypeError, "tzinfo.utcoffset() must return None or timedelta, not '%s'", Py_TYPE(delta)->tp_name);
					Py_DECREF(delta);
					Encoder_RETURN_FALSE;
				}
			} else {
				Py_DECREF(tzinfo);
			}

			Encoder_AppendFast('"');

			Encoder_RETURN_TRUE;
		}

		template<bool isDictKey>
		Encoder_FN(EncodeEnum) {
			PyObject* value = PyObject_GetAttr(obj, Module::State()->STR_VALUE);
			if (value != NULL) {
				bool res;
				if (isDictKey) {
					res = __encode_dict_key(value);
				} else {
					res = Encode(value);
				}
				Py_DECREF(value);
				return res;
			} else {
				Encoder_RETURN_FALSE;
			}
		}

		Encoder_FN(EncodeUUID) {
			PyObject* str = PyObject_Str(obj);
			if (str != NULL) {
				bool res = EncodeString(str);
				Py_DECREF(str);
				return res;
			} else {
				Encoder_RETURN_FALSE;
			}
		}

		Encoder_FN(EncodeDict) {
			Encoder_EnsureCapacity(Encoder_EXTRA_CAPACITY);
			Encoder_AppendFast('{');

			if (PyDict_Size(obj) == 0) {
				Encoder_AppendFast('}');
				Encoder_RETURN_TRUE;
			}

			Encoder_EnterRecursive();

			PyObject* key;
			PyObject* value;
			Py_ssize_t pos = 0;

			while (PyDict_Next(obj, &pos, &key, &value)) {
				Encoder_AppendFast('"');
				IF_LIKELY (__encode_dict_key(key)) {
					Encoder_AppendFast('"');
					Encoder_AppendFast(':');
					IF_LIKELY (Encode(value)) {
						Encoder_AppendFast(',');
					} else Encoder_HandleRecursion(YapicJson_Err_MaxRecursion_DictValue, value, key)
					} else {
						Encoder_RETURN_FALSE;
					}
				} else Encoder_HandleRecursion(YapicJson_Err_MaxRecursion_DictKey, key)
				} else {
					Encoder_RETURN_FALSE;
				}
			}

			--buffer.cursor; // overwrite last ','
			Encoder_AppendFast('}');
			Encoder_LeaveRecursive();
			Encoder_RETURN_TRUE;
		}

		Encoder_FN(EncodeItemsView) {
			Encoder_EnsureCapacity(Encoder_EXTRA_CAPACITY);
			Encoder_AppendFast('{');

			PyObject* iterator = PyObject_GetIter(obj);
			if (!iterator) {
				Encoder_RETURN_FALSE;
			}

			PyObject* item = NULL;
			PyObject *key;
			PyObject *value;
			Py_ssize_t length = 0;

			while ((item = PyIter_Next(iterator))) {
				IF_LIKELY (PyTuple_CheckExact(item) && PyTuple_GET_SIZE(item) == 2) {
					key = PyTuple_GET_ITEM(item, 0);
					value = PyTuple_GET_ITEM(item, 1);

					Encoder_AppendFast('"');
					IF_LIKELY (__encode_dict_key(key)) {
						Encoder_AppendFast('"');
						Encoder_AppendFast(':');
						IF_LIKELY (Encode(value)) {
							Encoder_AppendFast(',');
							++length;
						} else Encoder_HandleRecursion(YapicJson_Err_MaxRecursion_ItemsViewValue, value, key)
						} else {
							goto error;
						}
					} else Encoder_HandleRecursion(YapicJson_Err_MaxRecursion_ItemsViewKey, key)
					} else {
						goto error;
					}
				} else {
					PyErr_Format(Module::State()->EncodeError, YapicJson_Err_ItemsViewTuple, item);
					goto error;
				}
				Py_DECREF(item);
			}
			Py_DECREF(iterator);

			if (PyErr_Occurred()) {
				goto error;
			}

			if (length > 0) {
				--buffer.cursor; // overwrite last ','
			}

			Encoder_AppendFast('}');
			Encoder_LeaveRecursive();
			Encoder_RETURN_TRUE;

			error:
				Py_DECREF(iterator);
				Py_XDECREF(item);
				Encoder_RETURN_FALSE;
		}

		Encoder_FN(EncodeList) {
			Encoder_EnterRecursive();
			Encoder_EnsureCapacity(Encoder_EXTRA_CAPACITY);
			Encoder_AppendFast('[');

			Py_ssize_t length = PyList_GET_SIZE(obj);
			Py_ssize_t i = 0;

			for (; i<length ; i++) {
				if (Encode(PyList_GET_ITEM(obj, i))) {
					Encoder_AppendFast(',');
				} else Encoder_HandleRecursion(YapicJson_Err_MaxRecursion_ListValue, PyList_GET_ITEM(obj, i), i)
				} else {
					Encoder_RETURN_FALSE;
				}
			}

			if (length > 0) {
				--buffer.cursor; // overwrite last ','
			}

			Encoder_AppendFast(']');
			Encoder_LeaveRecursive();
			Encoder_RETURN_TRUE;
		}

		Encoder_FN(EncodeTuple) {
			Encoder_EnterRecursive();
			Encoder_EnsureCapacity(Encoder_EXTRA_CAPACITY);
			Encoder_AppendFast('[');

			Py_ssize_t length = PyTuple_GET_SIZE(obj);
			Py_ssize_t i = 0;

			for (; i<length ; i++) {
				if (Encode(PyTuple_GET_ITEM(obj, i))) {
					Encoder_AppendFast(',');
				} else Encoder_HandleRecursion(YapicJson_Err_MaxRecursion_ListValue, PyTuple_GET_ITEM(obj, i), i)
				} else {
					Encoder_RETURN_FALSE;
				}
			}

			if (length > 0) {
				--buffer.cursor; // overwrite last ','
			}

			Encoder_AppendFast(']');
			Encoder_LeaveRecursive();
			Encoder_RETURN_TRUE;
		}

		Encoder_FN(EncodeIterable) {
			Encoder_EnsureCapacity(Encoder_EXTRA_CAPACITY);
			Encoder_AppendFast('[');
			Encoder_EnterRecursive();

			PyObject *iterator = PyObject_GetIter(obj);
			if (iterator == NULL) {
				Encoder_RETURN_FALSE;
			}

			PyObject *item;
			Py_ssize_t length = 0;

			while ((item = PyIter_Next(iterator))) {
				IF_LIKELY (Encode(item)) {
					Py_DECREF(item);
					Encoder_AppendFast(',');
					++length;
				} else {
					if (Encoder_RecursionOccured()) {
						Encoder_RecursionError(YapicJson_Err_MaxRecursion_IterValue, item, length);
					}
					Py_DECREF(iterator);
					Py_DECREF(item);
					Encoder_RETURN_FALSE;
				}
			}
			Py_DECREF(iterator);

			if (PyErr_Occurred()) {
				Encoder_RETURN_FALSE;
			}

			if (length > 0) {
				--buffer.cursor; // overwrite last ','
			}

			Encoder_AppendFast(']');
			Encoder_LeaveRecursive();
			Encoder_RETURN_TRUE;
		}

		template<bool isDictKey>
		Encoder_FN(EncodeWithDefault) {
			Encoder_EnterRecursive();
			PyObject *toJson = PyObject_CallFunctionObjArgs(defaultFn, obj, NULL);

			IF_UNLIKELY (toJson == NULL) {
				Encoder_RETURN_FALSE;
			}

			IF_LIKELY ((isDictKey ? __encode_dict_key(toJson) : Encode(toJson))) {
				Py_DECREF(toJson);
				Encoder_LeaveRecursive();
				Encoder_RETURN_TRUE;
			} else {
				Py_DECREF(toJson);
				if (Encoder_RecursionOccured()) {
					Encoder_RecursionError(YapicJson_Err_MaxRecursion_Default, obj);
				}
			}

			Encoder_RETURN_FALSE;
		}

		template<bool isDictKey>
		Encoder_FN(EncodeWithJsonMethod) {
			Encoder_EnterRecursive();
			PyObject *toJson = PyObject_CallMethodObjArgs(obj, toJsonMethodName, NULL);

			IF_UNLIKELY (toJson == NULL) {
				Encoder_RETURN_FALSE;
			}

			IF_LIKELY ((isDictKey ? __encode_dict_key(toJson) : Encode(toJson))) {
				Py_DECREF(toJson);
				Encoder_LeaveRecursive();
				Encoder_RETURN_TRUE;
			} else {
				Py_DECREF(toJson);
				if (Encoder_RecursionOccured()) {
					Encoder_RecursionError(YapicJson_Err_MaxRecursion_JsonMethod, obj, toJsonMethodName);
				}
			}

			Encoder_RETURN_FALSE;
		}
};


} /* end namespace Json */
} /* end namespace Yapic */

#endif /* DEFBD7C4_2133_C63D_128D_F5D3D59111EF */
