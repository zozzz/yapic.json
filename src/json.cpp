#include "config.h"
#include "json.h"
#include "error.h"
#include "encoder.h"
#include "homogene_list.h"
#include "buffer.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"


#define __encode_new_encoder(__encoder, __buffer, __type, __ensure_ascii) \
	__encoder< __buffer<__type, YAPIC_JSON_ENCODER_BUFFER_SIZE>, __ensure_ascii > encoder; \
	encoder.defaultFn = defaultFn; \
	encoder.toJsonMethodName = toJsonMethodName; \
	encoder.encodeDatetime = encodeDatetime; \
	encoder.maxRecursionDepth = MAXIMUM_RECURSION_DEPTH


#include "decoder.h"

#define __decoder_options \
	decoder.objectHook = objectHook; \
	decoder.parseFloat = parseFloat; \
	decoder.parseDate = parseDate;

#define __decode_str(T) { \
	StrDecoder<T, Py_UCS4> decoder((T*) PyUnicode_DATA(input), PyUnicode_GET_LENGTH(input)); \
	__decoder_options \
	return decoder.Decode(); \
	}

#define __decode_bytes(T) { \
	BytesDecoder<T, Py_UCS4, YAPIC_JSON_BYTES_DECODER_BUFFER_SIZE> decoder((T*) PyBytes_AS_STRING(input), PyBytes_GET_SIZE(input)); \
	__decoder_options \
	return decoder.Decode(); \
	}

#define __decode_bytearray(T) { \
	BytesDecoder<T, Py_UCS4, YAPIC_JSON_BYTES_DECODER_BUFFER_SIZE> decoder((T*) PyByteArray_AS_STRING(input), PyByteArray_GET_SIZE(input)); \
	__decoder_options \
	return decoder.Decode(); \
	}


namespace Yapic { namespace Json {

	PyObject* Module::dumps(PyObject *module, PyObject *args, PyObject *kwargs) {
		static char* kwlist[] = {"obj", "ensure_ascii", "default", "tojson", "encode_datetime", NULL};

		PyObject* obj = NULL;
		PyObject* defaultFn = NULL;
		PyObject* toJsonMethodName = Module::State(module)->STR_TOJSON;
		bool ensureAscii = true;
		bool encodeDatetime = true;

		IF_LIKELY (PyArg_ParseTupleAndKeywords(args, kwargs, "O|bOUb", kwlist,
			&obj, &ensureAscii, &defaultFn, &toJsonMethodName, &encodeDatetime)) {

			if (ensureAscii == true) {
				__encode_new_encoder(Encoder, MemoryBuffer, Py_UCS1, true);
				IF_LIKELY (encoder.Encode(obj)) {
					return encoder.buffer.NewString();
				}
			} else {
				__encode_new_encoder(Encoder, MemoryBuffer, Py_UCS4, false);
				IF_LIKELY (encoder.Encode(obj)) {
					return encoder.buffer.NewString();
				}
			}
		}

		return NULL;
	}

	PyObject* Module::dumpb(PyObject *module, PyObject *args, PyObject *kwargs) {
		static char* kwlist[] = {"obj", "ensure_ascii", "default", "tojson", "encode_datetime", NULL};

		PyObject* obj = NULL;
		PyObject* defaultFn = NULL;
		PyObject* toJsonMethodName = Module::State(module)->STR_TOJSON;
		bool ensureAscii = true;
		bool encodeDatetime = true;

		IF_LIKELY (PyArg_ParseTupleAndKeywords(args, kwargs, "O|bOUb", kwlist,
			&obj, &ensureAscii, &defaultFn, &toJsonMethodName, &encodeDatetime)) {

			if (ensureAscii == true) {
				__encode_new_encoder(Encoder, MemoryBuffer, Py_UCS1, true);
				IF_LIKELY (encoder.Encode(obj)) {
					return encoder.buffer.NewBytes();
				}
			} else {
				__encode_new_encoder(Encoder, MemoryBuffer, Py_UCS1, false);
				IF_LIKELY (encoder.Encode(obj)) {
					return encoder.buffer.NewBytes();
				}
			}
		}

		return NULL;
	}


	PyObject* Module::dump(PyObject *module, PyObject *args, PyObject *kwargs) {
		static char* kwlist[] = {"obj", "fp", "ensure_ascii", "default", "tojson", "encode_datetime", "encode_homogene", NULL};

		PyObject* obj = NULL;
		PyObject* defaultFn = NULL;
		PyObject* toJsonMethodName = Module::State(module)->STR_TOJSON;
		PyObject* fp = NULL;
		bool ensureAscii = true;
		bool encodeDatetime = true;
		bool encodeHomogene = true;

		IF_LIKELY (PyArg_ParseTupleAndKeywords(args, kwargs, "OO|bOUb", kwlist,
			&obj, &fp, &ensureAscii, &defaultFn, &toJsonMethodName, &encodeDatetime, &encodeHomogene)) {

			if (ensureAscii == true) {
				__encode_new_encoder(Encoder, FileBuffer, Py_UCS1, true);
				IF_LIKELY (encoder.buffer.SetTarget(fp) && encoder.Encode(obj) && encoder.buffer.Flush()) {
					Py_RETURN_NONE;
				}
			} else {
				__encode_new_encoder(Encoder, FileBuffer, Py_UCS4, false);
				IF_LIKELY (encoder.buffer.SetTarget(fp) && encoder.Encode(obj) && encoder.buffer.Flush()) {
					Py_RETURN_NONE;
				}
			}
		}

		return NULL;
	}


	PyObject* Module::loads(PyObject *module, PyObject *args, PyObject *kwargs) {
		static char* kwlist[] = {"s", "object_hook", "parse_float", "parse_date", NULL};

		PyObject* input;
		PyObject* objectHook = NULL;
		PyObject* parseFloat = NULL;
		bool parseDate = true;

		IF_LIKELY (PyArg_ParseTupleAndKeywords(args, kwargs, "O|OOb", kwlist,
			&input, &objectHook, &parseFloat, &parseDate)) {

			if (objectHook != NULL && !PyCallable_Check(objectHook)) {
				PyErr_SetString(PyExc_TypeError, "argument 'object_hook' must be callable");
			}

			if (parseFloat != NULL && !PyCallable_Check(parseFloat)) {
				PyErr_SetString(PyExc_TypeError, "argument 'parse_float' must be callable");
			}

			if (PyUnicode_Check(input)) {
				switch (PyUnicode_KIND(input)) {
					case PyUnicode_1BYTE_KIND:
						__decode_str(Py_UCS1);
					break;

					case PyUnicode_2BYTE_KIND:
						__decode_str(Py_UCS2);
					break;

					case PyUnicode_4BYTE_KIND:
						__decode_str(Py_UCS4);
					break;

					default:
						assert(0);
					break;
				}
			} else if (PyBytes_Check(input)) {
				__decode_bytes(Py_UCS1);
			} else if (PyByteArray_Check(input)) {
				__decode_bytearray(Py_UCS1);
			} else {
				PyErr_SetString(PyExc_TypeError, "argument 1 must be str or bytes or bytearray");
			}
		}

		return NULL;
	}

} /* end namespace Json */
} /* end namespace Yapic */


PyMODINIT_FUNC PyInit__json(void) {
	return Yapic::Json::Module::Create();
}

// // static PyObject *YapicJson_load(PyObject *self, PyObject *args, PyObject *kwargs) {
// // 	Py_RETURN_NONE;
// // }


// static PyMethodDef YapicJson_Methods[] = {
//     {"loads", (PyCFunction) YapicJson_loads, METH_VARARGS | METH_KEYWORDS, ""},
//     // {"load", (PyCFunction) YapicJson_load, METH_VARARGS | METH_KEYWORDS, ""},
//     {"dumps", (PyCFunction) YapicJson_dumps, METH_VARARGS | METH_KEYWORDS, ""},
//     {"dump", (PyCFunction) YapicJson_dump, METH_VARARGS | METH_KEYWORDS, ""},
// 	{NULL, NULL, 0, NULL}
// };


// static struct PyModuleDef YapicJson_Module = {
// 	PyModuleDef_HEAD_INIT,
// 	"yapic.json",
// 	"",
// 	-1,
// 	YapicJson_Methods
// };


// PyMODINIT_FUNC PyInit_json(void) {
// 	if (PyType_Ready(&Yapic::Json::HomogeneList_Type) < 0) {
// 		return NULL;
// 	}

// 	PyObject *module = PyModule_Create(&YapicJson_Module);
// 	if (module == NULL) {
// 		return NULL;
// 	}

// 	PyDateTime_IMPORT;

// 	PyObject* datetime = PyImport_ImportModule("datetime");
// 	if (datetime == NULL) {
// 		return NULL;
// 	}

// 	Yapic::Json::PyTimezone = PyObject_GetAttrString(datetime, "timezone");
// 	if (Yapic::Json::PyTimezone == NULL) {
// 		Py_DECREF(datetime);
// 		return NULL;
// 	}

// 	Yapic::Json::PyUTCTimezone = PyObject_GetAttrString(Yapic::Json::PyTimezone, "utc");
// 	if (Yapic::Json::PyUTCTimezone == NULL) {
// 		Py_DECREF(Yapic::Json::PyTimezone);
// 		Py_DECREF(datetime);
// 		return NULL;
// 	}

// 	PyModule_AddStringConstant(module, "__version__", YAPIC_JSON_VERSION_STR);

// 	Yapic::Json::Error = PyErr_NewException("yapic.json.JsonError", NULL, NULL);
// 	Py_INCREF(Yapic::Json::Error);

// 	Yapic::Json::DecodeError = PyErr_NewException("yapic.json.JsonDecodeError", Yapic::Json::Error, NULL);
// 	Py_INCREF(Yapic::Json::DecodeError);

// 	Yapic::Json::EncodeError = PyErr_NewException("yapic.json.JsonEncodeError", Yapic::Json::Error, NULL);
// 	Py_INCREF(Yapic::Json::EncodeError);

// 	Yapic::Json::TO_JSON_DEFAULT_METHOD_NAME = PyUnicode_InternFromString("__json__");
// 	Yapic::Json::TZINFO_NAME = PyUnicode_InternFromString("tzinfo");
// 	Yapic::Json::UTCOFFSET_METHOD_NAME = PyUnicode_InternFromString("utcoffset");
// 	Yapic::Json::WRITE_METHOD_NAME = PyUnicode_InternFromString("write");

// 	PyModule_AddObject(module, "JsonError", Yapic::Json::Error);
// 	PyModule_AddObject(module, "JsonDecodeError", Yapic::Json::DecodeError);
// 	PyModule_AddObject(module, "JsonEncodeError", Yapic::Json::EncodeError);

// 	Py_INCREF(&Yapic::Json::HomogeneList_Type);
//     PyModule_AddObject(module, "HomogeneList", (PyObject*)&Yapic::Json::HomogeneList_Type);

// 	return module;
// }
