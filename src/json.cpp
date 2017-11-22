#include "config.h"
#include "json.h"
#include "error.h"
#include "encoder.h"
#include "homogene_list.h"
#include "buffer.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"


#define __encode_new_encoder(B, T) \
	Encoder< B<T, YAPIC_JSON_ENCODER_BUFFER_SIZE> > encoder; \
	encoder.defaultFn = defaultFn; \
	encoder.toJsonMethodName = toJsonMethodName; \
	encoder.encodeDatetime = encodeDatetime; \
	encoder.encodeHomogene = encodeHomogene; \
	encoder.maxRecursionDepth = MAXIMUM_RECURSION_DEPTH


#include "decoder.h"

#define __decode(T) { \
	Decoder<T, Py_UCS4> decoder(\
		(T*) PyUnicode_DATA(input), \
		PyUnicode_GET_LENGTH(input)); \
	decoder.objectHook = objectHook; \
	decoder.parseFloat = parseFloat; \
	decoder.parseDate = parseDate; \
	return decoder.Decode(); \
	}


namespace Yapic { namespace Json {

	PyObject* Module::dumps(PyObject *module, PyObject *args, PyObject *kwargs) {
		static char* kwlist[] = {"obj", "ensure_ascii", "default", "tojson", "encode_datetime", "encode_homogene", NULL};

		PyObject* obj = NULL;
		PyObject* defaultFn = NULL;
		PyObject* toJsonMethodName = Module::State(module)->STR_TOJSON;
		bool ensureAscii = true;
		bool encodeDatetime = true;
		bool encodeHomogene = true;

		if (EXPECT_TRUE(PyArg_ParseTupleAndKeywords(args, kwargs, "O|bOUb", kwlist,
			&obj, &ensureAscii, &defaultFn, &toJsonMethodName, &encodeDatetime, &encodeHomogene))) {

			if (ensureAscii == true) {
				__encode_new_encoder(MemoryBuffer, Py_UCS1);
				if (EXPECT_TRUE(encoder.Encode(obj))) {
					return encoder.buffer.NewString();
				}
			} else {
				__encode_new_encoder(MemoryBuffer, Py_UCS4);
				if (EXPECT_TRUE(encoder.Encode(obj))) {
					return encoder.buffer.NewString();
				}
			}
		}

		return NULL;
	}


	PyObject* Module::dumpb(PyObject *module, PyObject *args, PyObject *kwargs) {
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

		if (EXPECT_TRUE(PyArg_ParseTupleAndKeywords(args, kwargs, "OO|bOUb", kwlist,
			&obj, &fp, &ensureAscii, &defaultFn, &toJsonMethodName, &encodeDatetime, &encodeHomogene))) {

			if (ensureAscii == true) {
				__encode_new_encoder(FileBuffer, Py_UCS1);
				if (EXPECT_TRUE(encoder.buffer.SetTarget(fp) && encoder.Encode(obj) && encoder.buffer.Flush())) {
					Py_RETURN_NONE;
				}
			} else {
				__encode_new_encoder(FileBuffer, Py_UCS4);
				if (EXPECT_TRUE(encoder.buffer.SetTarget(fp) && encoder.Encode(obj) && encoder.buffer.Flush())) {
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

		if (EXPECT_TRUE(PyArg_ParseTupleAndKeywords(args, kwargs, "U|OOb", kwlist,
			&input, &objectHook, &parseFloat, &parseDate))) {

			if (objectHook != NULL && !PyCallable_Check(objectHook)) {
				PyErr_SetString(PyExc_TypeError, "argument 'object_hook' must be callable");
			}

			if (parseFloat != NULL && !PyCallable_Check(parseFloat)) {
				PyErr_SetString(PyExc_TypeError, "argument 'parse_float' must be callable");
			}

			switch (PyUnicode_KIND(input)) {
				case PyUnicode_1BYTE_KIND:
					__decode(Py_UCS1);
				break;

				case PyUnicode_2BYTE_KIND:
					__decode(Py_UCS2);
				break;

				case PyUnicode_4BYTE_KIND:
					__decode(Py_UCS4);
				break;

				default:
					assert(0);
				break;
			}
		}

		return NULL;
	}

} /* end namespace Json */
} /* end namespace Yapic */


PyMODINIT_FUNC PyInit_json(void) {
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