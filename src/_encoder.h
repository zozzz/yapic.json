#ifndef CA3F9791_E133_C640_147F_AAE8822A9118
#define CA3F9791_E133_C640_147F_AAE8822A9118

#include <type_traits>
#include "config.h"
#include "buffer.h"
#include "str_encode_table.h"
#include "error.h"
#include "dtoa_milo.h"

#define ZiboJson_Encoder_EXTRA_CAPACITY 10

#define ZiboJson_EncBuffer_EnsureCapacity(buffer, additional_size) \
	ZiboJson_Buffer_EnsureCapacity(buffer, (additional_size) + ZiboJson_Encoder_EXTRA_CAPACITY, false)

#define _

/**
 * Recursion checking helper.
 * usage:
 *
 * ZiboJson_Encoder_EnterRecursiveCall("[why enter recursive call message]");
 * function call...
 * ZiboJson_Encoder_LeaveRecursiveCall();
 */
#define ZiboJson_Encoder_EnterRecursiveCall(msg, ...) \
	if (++encoder->recursionDepth > encoder->maxRecursionDepth) { \
		PyErr_Format(EncodeError, "Maximum recursion level reached, while " msg, __VA_ARGS__); \
		return false; \
	}

#define ZiboJson_Encoder_LeaveRecursiveCall() --encoder->recursionDepth

template<typename T, typename B>
struct _ZiboJson_Encoder {
	typedef T Char;
	typedef B Buffer;

	B buffer;
	PyObject* defaultFn;
	PyObject* toJsonMethodName;
	int maxRecursionDepth;
	int recursionDepth;
	bool encodeDatetime;
};

template<typename T>
struct ZiboJson_Encoder {
	typedef _ZiboJson_Encoder<T, ZiboJson_FileBuffer<T>> ToFile;
	typedef _ZiboJson_Encoder<T, ZiboJson_Buffer<T>> ToString;
};


//////////////////////////////////////////////////////////////////////////////
// ENCODE STRING
//////////////////////////////////////////////////////////////////////////////

#define HEX_CHAR(idx) ("0123456789abcdef"[(idx)])

#define STRING_ENC_COMMON_CASES \
	STR_ASCII_CASE: ZiboJson_Buffer_AppendFast(*buffer, (BUFF::Char) *cursor); break; \
	STR_ESC_DQUOTE_CASE: ZiboJson_Buffer_AppendFast(*buffer, '\\'); ZiboJson_Buffer_AppendFast(*buffer, '"'); break; \
	STR_ESC_BSLASH_CASE: ZiboJson_Buffer_AppendFast(*buffer, '\\'); ZiboJson_Buffer_AppendFast(*buffer, '\\'); break; \
	STR_ESC_CR_CASE: ZiboJson_Buffer_AppendFast(*buffer, '\\'); ZiboJson_Buffer_AppendFast(*buffer, 'r'); break; \
	STR_ESC_LF_CASE: ZiboJson_Buffer_AppendFast(*buffer, '\\'); ZiboJson_Buffer_AppendFast(*buffer, 'n'); break; \
	STR_ESC_TAB_CASE: ZiboJson_Buffer_AppendFast(*buffer, '\\'); ZiboJson_Buffer_AppendFast(*buffer, 't'); break; \
	STR_ESC_BS_CASE: ZiboJson_Buffer_AppendFast(*buffer, '\\'); ZiboJson_Buffer_AppendFast(*buffer, 'b'); break; \
	STR_ESC_FF_CASE: ZiboJson_Buffer_AppendFast(*buffer, '\\'); ZiboJson_Buffer_AppendFast(*buffer, 'f'); break; \
	STR_NULL_CASE: \
		ZiboJson_Buffer_AppendFast(*buffer, '\\'); \
		ZiboJson_Buffer_AppendFast(*buffer, 'u'); \
		ZiboJson_Buffer_AppendFast(*buffer, '0'); \
		ZiboJson_Buffer_AppendFast(*buffer, '0'); \
		ZiboJson_Buffer_AppendFast(*buffer, '0'); \
		ZiboJson_Buffer_AppendFast(*buffer, '0'); \
	break

#define STRING_TO_UNICODE_ENC_COMMON_CASES \
	STRING_ENC_COMMON_CASES; \
	STR_UNPRINTABLE_ESC_CASE: \
		ZiboJson_Buffer_AppendFast(*buffer, '\\'); \
		ZiboJson_Buffer_AppendFast(*buffer, 'u'); \
		ZiboJson_Buffer_AppendFast(*buffer, '0'); \
		ZiboJson_Buffer_AppendFast(*buffer, '0'); \
		ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR(((*cursor) & 0xF0) >> 4)); \
		ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR(((*cursor) & 0x0F))); \
	break

#define STRING_ENCODER_FN_FROM_ASCII \
	std::is_same<CHIN, Py_UCS1>::value

#define STRING_ENCODER_FN_FROM_UNICODE \
	(std::is_same<CHIN, Py_UCS2>::value || std::is_same<CHIN, Py_UCS4>::value)

#define STRING_ENCODER_FN_TO_UNICODE \
	(std::is_base_of<ZiboJson_Buffer<Py_UCS2>, BUFF>::value || \
	 std::is_base_of<ZiboJson_Buffer<Py_UCS4>, BUFF>::value)

#define STRING_ENCODER_FN_TO_ASCII \
	std::is_base_of<ZiboJson_Buffer<Py_UCS1>, BUFF>::value

#define STRING_ENCODER_FN(from, to) \
	template<typename CHIN, typename BUFF> \
	bool __encode_string(register BUFF* buffer, register CHIN* cursor, register const CHIN* end, \
		typename std::enable_if<(STRING_ENCODER_FN_ ## from) && (STRING_ENCODER_FN_ ## to)>::type* = 0)

STRING_ENCODER_FN(FROM_ASCII, TO_ASCII) {
	while (cursor < end) {
		switch (*cursor) {
			STRING_ENC_COMMON_CASES;
			STR_ESC_DEL_CASE:
			STR_UNPRINTABLE_ESC_CASE:
			default:
				ZiboJson_Buffer_AppendFast(*buffer, '\\');
				ZiboJson_Buffer_AppendFast(*buffer, 'u');
				ZiboJson_Buffer_AppendFast(*buffer, '0');
				ZiboJson_Buffer_AppendFast(*buffer, '0');
				ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR(((*cursor) & 0xF0) >> 4));
				ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR(((*cursor) & 0x0F)));
			break;
		}
		++cursor;
	}
	return true;
}


STRING_ENCODER_FN(FROM_ASCII, TO_UNICODE) {
	register BUFF::Char maxchar = buffer->maxchar;
	while (cursor < end) {
		switch (*cursor) {
			STRING_TO_UNICODE_ENC_COMMON_CASES;

			STR_ESC_DEL_CASE:
			default:
				if (maxchar < *cursor) {
					buffer->maxchar = maxchar = *cursor;
				}
				ZiboJson_Buffer_AppendFast(*buffer, *cursor);
			break;
		}
		++cursor;
	}
	return true;
}


STRING_ENCODER_FN(FROM_UNICODE, TO_UNICODE) {
	register BUFF::Char maxchar = buffer->maxchar;
	while (cursor < end) {
		switch (*cursor) {
			STRING_TO_UNICODE_ENC_COMMON_CASES;
			default:
				if (maxchar < *cursor) {
					buffer->maxchar = maxchar = *cursor;
				}
				ZiboJson_Buffer_AppendFast(*buffer, *cursor);
			break;
		}
		++cursor;
	}
	return true;
}


STRING_ENCODER_FN(FROM_UNICODE, TO_ASCII) {
	while (cursor < end) {
		switch (*cursor) {
			STRING_ENC_COMMON_CASES;
			STR_UNPRINTABLE_ESC_CASE:
				ZiboJson_Buffer_AppendFast(*buffer, '\\');
				ZiboJson_Buffer_AppendFast(*buffer, 'u');
				ZiboJson_Buffer_AppendFast(*buffer, '0');
				ZiboJson_Buffer_AppendFast(*buffer, '0');
				ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR(((*cursor) & 0xF0) >> 4));
				ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR(((*cursor) & 0x0F)));
			break;

			default:
				ZiboJson_Buffer_AppendFast(*buffer, '\\');
				ZiboJson_Buffer_AppendFast(*buffer, 'u');
				if (*cursor > 0xFFFF) { 	// \uXXXX\uXXXX
					register unsigned int higher = ((*cursor) >> 10) - 0x40;
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( 0xD ));
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( 0x8 | ((higher >> 8) & 0x3) ));
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( (higher >> 4) & 0xF ));
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( higher & 0xF ));

					ZiboJson_Buffer_AppendFast(*buffer, '\\');
					ZiboJson_Buffer_AppendFast(*buffer, 'u');
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( 0xD ));
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( 0xC | ((*cursor >> 8) & 0x3) ));
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( (*cursor >> 4) & 0xF ));
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( *cursor & 0xF ));
				} else { 					// \uXXXX
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( (*cursor >> 12) ));
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( (*cursor >> 8) & 0xF ));
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( (*cursor >> 4) & 0xF ));
					ZiboJson_Buffer_AppendFast(*buffer, HEX_CHAR( (*cursor) & 0xF ));
				}
			break;
		}
		++cursor;
	}

	return true;
}


template<typename T>
inline bool zj_encode_string(T* encoder, PyObject* obj) {
	register Py_ssize_t length = PyUnicode_GET_LENGTH(obj);
	ZiboJson_EncBuffer_EnsureCapacity(encoder->buffer, length * 6);

	switch (PyUnicode_KIND(obj)) {
		case PyUnicode_1BYTE_KIND:
			return __encode_string(&encoder->buffer, PyUnicode_1BYTE_DATA(obj), PyUnicode_1BYTE_DATA(obj) + length);

		case PyUnicode_2BYTE_KIND:
			return __encode_string(&encoder->buffer, PyUnicode_2BYTE_DATA(obj), PyUnicode_2BYTE_DATA(obj) + length);

		case PyUnicode_4BYTE_KIND:
			return __encode_string(&encoder->buffer, PyUnicode_4BYTE_DATA(obj), PyUnicode_4BYTE_DATA(obj) + length);
	}
	// TODO: error
	return false;
}


template<typename T>
inline bool zj_encode_int(T* encoder, PyObject* obj) {
	ZiboJson_EncBuffer_EnsureCapacity(encoder->buffer, LONG_MAX_LENGTH_IN_CHR);

	register long value = PyLong_AS_LONG(obj);
	register unsigned long abs_value = value;

	if (value < 0) {
		abs_value = -value;
		ZiboJson_Buffer_AppendFast(encoder->buffer, '-');
	}

	register T::Buffer::Char* end_position = ZiboJson_Buffer_Cursor(encoder->buffer) + LONG_MAX_LENGTH_IN_CHR;
	register T::Buffer::Char* saved_end_position = end_position;

	do {
		*(--end_position) = (48 + (abs_value % 10));
	} while ((abs_value /= 10) > 0);

	abs_value = (saved_end_position - end_position);

	memcpy(ZiboJson_Buffer_Cursor(encoder->buffer), end_position, sizeof(T::Buffer::Char) * abs_value);
	ZiboJson_Buffer_Cursor(encoder->buffer) += abs_value;

	return true;
}


template<typename T>
inline bool zj_encode_float(T* encoder, PyObject* obj) {
	ZiboJson_EncBuffer_EnsureCapacity(encoder->buffer, 200);

	int length;

	dtoa_milo(
		PyFloat_AS_DOUBLE(obj),
		ZiboJson_Buffer_Cursor(encoder->buffer),
		&length
	);

	ZiboJson_Buffer_Cursor(encoder->buffer) += length;

	return true;
}


template<typename T>
inline bool zj_encode_dict_key(T* encoder, PyObject* obj);


template<typename T>
inline bool zj_encode_dict(T* encoder, PyObject* obj) {
	ZiboJson_Buffer_EnsureCapacity(encoder->buffer, ZiboJson_Encoder_EXTRA_CAPACITY, false);
	ZiboJson_Buffer_AppendFast(encoder->buffer, '{');

	if (PyDict_Size(obj) == 0) {
		ZiboJson_Buffer_AppendFast(encoder->buffer, '}');
		return true;
	}

	PyObject* key;
	PyObject* value;
	Py_ssize_t pos = 0;

	while (PyDict_Next(obj, &pos, &key, &value)) {
		ZiboJson_Encoder_EnterRecursiveCall("encoding dict key: %R", key);
		ZiboJson_Buffer_AppendFast(encoder->buffer, '"');

		if (zj_encode_dict_key(encoder, key)) {
			ZiboJson_Encoder_LeaveRecursiveCall();

			ZiboJson_Buffer_AppendFast(encoder->buffer, '"');
			ZiboJson_Buffer_AppendFast(encoder->buffer, ':');

			ZiboJson_Encoder_EnterRecursiveCall("encoding dict value: %R", value);

			if (ZiboJson_EncodeObject(encoder, value)) {
				ZiboJson_Encoder_LeaveRecursiveCall();
				ZiboJson_Buffer_AppendFast(encoder->buffer, ',');
			} else {
				return false;
			}
		} else {
			return false;
		}
	}

	--ZiboJson_Buffer_Cursor(encoder->buffer); // overwrite last ','
	ZiboJson_Buffer_AppendFast(encoder->buffer, '}');
	ZiboJson_Buffer_EnsureCapacity(encoder->buffer, ZiboJson_Encoder_EXTRA_CAPACITY, false);
	return true;
}


template<typename T>
inline bool zj_encode_list(T* encoder, PyObject* obj) {
	ZiboJson_Buffer_EnsureCapacity(encoder->buffer, ZiboJson_Encoder_EXTRA_CAPACITY, false);
	ZiboJson_Buffer_AppendFast(encoder->buffer, '[');

	register Py_ssize_t length = PyList_GET_SIZE(obj);
	register unsigned int i = 0;

	for (; i<length ; i++) {
		ZiboJson_Encoder_EnterRecursiveCall("encoding array item in %du position", i);
		if (ZiboJson_EncodeObject(encoder, PyList_GET_ITEM(obj, i))) {
			ZiboJson_Encoder_LeaveRecursiveCall();
			ZiboJson_Buffer_AppendFast(encoder->buffer, ',');
		} else {
			return false;
		}
	}

	if (length > 0) {
		--ZiboJson_Buffer_Cursor(encoder->buffer); // overwrite last ','
	}

	ZiboJson_Buffer_AppendFast(encoder->buffer, ']');
	ZiboJson_Buffer_EnsureCapacity(encoder->buffer, ZiboJson_Encoder_EXTRA_CAPACITY, false);
	return true;
}


template<typename T>
inline bool zj_encode_tuple(T* encoder, PyObject* obj) {
	ZiboJson_Buffer_EnsureCapacity(encoder->buffer, ZiboJson_Encoder_EXTRA_CAPACITY, false);
	ZiboJson_Buffer_AppendFast(encoder->buffer, '[');

	register Py_ssize_t length = PyTuple_GET_SIZE(obj);
	register unsigned int i = 0;

	for (; i<length ; i++) {
		ZiboJson_Encoder_EnterRecursiveCall("encoding array item in %du position", i);
		if (ZiboJson_EncodeObject(encoder, PyTuple_GET_ITEM(obj, i))) {
			ZiboJson_Encoder_LeaveRecursiveCall();
			ZiboJson_Buffer_AppendFast(encoder->buffer, ',');
		} else {
			return false;
		}
	}

	if (length > 0) {
		--ZiboJson_Buffer_Cursor(encoder->buffer); // overwrite last ','
	}

	ZiboJson_Buffer_AppendFast(encoder->buffer, ']');
	ZiboJson_Buffer_EnsureCapacity(encoder->buffer, ZiboJson_Encoder_EXTRA_CAPACITY, false);
	return true;
}


template<typename T>
inline bool zj_encode_iterable(T* encoder, PyObject* obj) {
	ZiboJson_Buffer_EnsureCapacity(encoder->buffer, ZiboJson_Encoder_EXTRA_CAPACITY, false);
	ZiboJson_Buffer_AppendFast(encoder->buffer, '[');

	register PyObject *iterator = PyObject_GetIter(obj);
	register PyObject *item;
	register Py_ssize_t length = 0;

	if (iterator == NULL) {
		return false;
	}

	while (item = PyIter_Next(iterator)) {
		ZiboJson_Encoder_EnterRecursiveCall("encoding item: %R of the iterable object: %R", item, obj);
		if (!ZiboJson_EncodeObject(encoder, item)) {
			Py_DECREF(item);
			return false;
		}
		ZiboJson_Encoder_LeaveRecursiveCall();
		Py_DECREF(item);

		ZiboJson_Buffer_AppendFast(encoder->buffer, ',');
		++length;
	}

	if (length > 0) {
		--ZiboJson_Buffer_Cursor(encoder->buffer); // overwrite last ','
	}

	Py_DECREF(iterator);

	if (PyErr_Occurred()) {
		return false;
	}

	ZiboJson_Buffer_AppendFast(encoder->buffer, ']');
	ZiboJson_Buffer_EnsureCapacity(encoder->buffer, ZiboJson_Encoder_EXTRA_CAPACITY, false);
	return true;
}


template<typename T>
inline bool zj_encode_dict_key(T* encoder, PyObject* obj) {
	if (PyUnicode_CheckExact(obj)) {
		return zj_encode_string(encoder, obj);
	} else if (PyLong_CheckExact(obj)) {
		return zj_encode_int(encoder, obj);
	} else if (PyFloat_CheckExact(obj)) {
		return zj_encode_float(encoder, obj);
	} else if (obj == Py_True) {
		ZiboJson_EncBuffer_EnsureCapacity(encoder->buffer, 4);
		ZiboJson_Buffer_AppendFast(encoder->buffer, 't');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'r');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'u');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'e');
		return true;
	} else if (obj == Py_False) {
		ZiboJson_EncBuffer_EnsureCapacity(encoder->buffer, 5);
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'f');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'a');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'l');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 's');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'e');
		return true;
	} else if (obj == Py_None) {
		ZiboJson_EncBuffer_EnsureCapacity(encoder->buffer, 4);
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'n');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'u');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'l');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'l');
		return true;
	}

	PyErr_Clear();
	PyErr_Format(EncodeError, "This %R is an invalid dict key, please provide the 'default' function or define the %A method in class.", obj, encoder->toJsonMethodName);
	return false;
}


template<typename T>
bool ZiboJson_EncodeObject(T* encoder, PyObject* obj) {
	if (PyUnicode_CheckExact(obj)) {
		ZiboJson_Buffer_AppendFast(encoder->buffer, '"');
		if (zj_encode_string(encoder, obj)) {
			ZiboJson_Buffer_AppendFast(encoder->buffer, '"');
			return true;
		}
	} else if (PyDict_CheckExact(obj)) {
		return zj_encode_dict(encoder, obj);
	} else if (PyList_CheckExact(obj)) {
		return zj_encode_list(encoder, obj);
	} else if (PyTuple_CheckExact(obj)) {
		return zj_encode_tuple(encoder, obj);
	} else if (PyLong_CheckExact(obj)) {
		return zj_encode_int(encoder, obj);
	} else if (PyFloat_CheckExact(obj)) {
		return zj_encode_float(encoder, obj);
	} else if (obj == Py_True) {
		ZiboJson_EncBuffer_EnsureCapacity(encoder->buffer, 4);
		ZiboJson_Buffer_AppendFast(encoder->buffer, 't');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'r');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'u');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'e');
		return true;
	} else if (obj == Py_False) {
		ZiboJson_EncBuffer_EnsureCapacity(encoder->buffer, 5);
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'f');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'a');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'l');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 's');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'e');
		return true;
	} else if (obj == Py_None) {
		ZiboJson_EncBuffer_EnsureCapacity(encoder->buffer, 4);
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'n');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'u');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'l');
		ZiboJson_Buffer_AppendFast(encoder->buffer, 'l');
		return true;
	} else if (PyAnySet_Check(obj)) {
		return zj_encode_iterable(encoder, obj);
	}

	return false;
}

#endif /* CA3F9791_E133_C640_147F_AAE8822A9118 */
