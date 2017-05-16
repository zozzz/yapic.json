#include "config.h"
#include "globals.h"
#include "error.h"
#include "encoder.h"
#include "homogene_list.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"


#define __encode_new_encoder(B, T) \
	ZiboJson::Encoder< ZiboJson::B<T, ZIBO_JSON_ENCODER_BUFFER_SIZE> > encoder; \
	encoder.defaultFn = defaultFn; \
	encoder.toJsonMethodName = toJsonMethodName; \
	encoder.encodeDatetime = encodeDatetime; \
	encoder.encodeHomogene = encodeHomogene; \
	encoder.maxRecursionDepth = MAXIMUM_RECURSION_DEPTH


static PyObject *ZiboJson_dumps(PyObject *self, PyObject *args, PyObject *kwargs) {
	static char* kwlist[] = {"obj", "ensure_ascii", "default", "tojson", "encode_datetime", "encode_homogene", NULL};

	PyObject* obj = NULL;
	PyObject* defaultFn = NULL;
	PyObject* toJsonMethodName = ZiboJson::TO_JSON_DEFAULT_METHOD_NAME;
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


static PyObject *ZiboJson_dump(PyObject *self, PyObject *args, PyObject *kwargs) {
	static char* kwlist[] = {"obj", "fp", "ensure_ascii", "default", "tojson", "encode_datetime", "encode_homogene", NULL};

	PyObject* obj = NULL;
	PyObject* defaultFn = NULL;
	PyObject* toJsonMethodName = ZiboJson::TO_JSON_DEFAULT_METHOD_NAME;
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


#define NEW_PARSER 1

#if NEW_PARSER

	#include "decoder.h"

	#define __decode(T) { \
		ZiboJson::Decoder<T, ZiboJson::MemoryBuffer<Py_UCS4, ZIBO_JSON_DECODER_BUFFER_SIZE> > decoder(\
			(T*) PyUnicode_DATA(input), \
			PyUnicode_GET_LENGTH(input)); \
		decoder.objectHook = objectHook; \
		decoder.parseFloat = parseFloat; \
		decoder.parseDate = parseDate; \
		return decoder.Decode(); \
		}
#else

	#include "_decoder.h"

	#define __decode(T) { \
		ZiboJson::Decoder<T, ZiboJson::MemoryBuffer<Py_UCS4, ZIBO_JSON_DECODER_BUFFER_SIZE> > decoder((T*) data, length); \
		decoder.objectHook = objectHook; \
		decoder.parseFloat = parseFloat; \
		decoder.parseDate = parseDate; \
		return decoder.ReadValue(); \
		}
#endif


static PyObject *ZiboJson_loads(PyObject *self, PyObject *args, PyObject *kwargs) {
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

		#if NEW_PARSER == 0
		void* data = PyUnicode_DATA(input);
		size_t length = PyUnicode_GET_LENGTH(input);
		#endif

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

// static PyObject *ZiboJson_load(PyObject *self, PyObject *args, PyObject *kwargs) {
// 	Py_RETURN_NONE;
// }


static PyMethodDef ZiboJson_Methods[] = {
    {"loads", (PyCFunction) ZiboJson_loads, METH_VARARGS | METH_KEYWORDS, ""},
    // {"load", (PyCFunction) ZiboJson_load, METH_VARARGS | METH_KEYWORDS, ""},
    {"dumps", (PyCFunction) ZiboJson_dumps, METH_VARARGS | METH_KEYWORDS, ""},
    {"dump", (PyCFunction) ZiboJson_dump, METH_VARARGS | METH_KEYWORDS, ""},
	{NULL, NULL, 0, NULL}
};


static struct PyModuleDef ZiboJson_Module = {
	PyModuleDef_HEAD_INIT,
	"zibo.json",
	"",
	-1,
	ZiboJson_Methods
};


PyMODINIT_FUNC PyInit_json(void) {
	if (PyType_Ready(&ZiboJson::HomogeneList_Type) < 0) {
		return NULL;
	}

	PyObject *module = PyModule_Create(&ZiboJson_Module);
	if (module == NULL) {
		return NULL;
	}

	PyDateTime_IMPORT;

	PyObject* datetime = PyImport_ImportModule("datetime");
	if (datetime == NULL) {
		return NULL;
	}

	ZiboJson::PyTimezone = PyObject_GetAttrString(datetime, "timezone");
	if (ZiboJson::PyTimezone == NULL) {
		Py_DECREF(datetime);
		return NULL;
	}

	ZiboJson::PyUTCTimezone = PyObject_GetAttrString(ZiboJson::PyTimezone, "utc");
	if (ZiboJson::PyUTCTimezone == NULL) {
		Py_DECREF(ZiboJson::PyTimezone);
		Py_DECREF(datetime);
		return NULL;
	}

	PyModule_AddStringConstant(module, "__version__", ZIBO_JSON_VERSION_STR);

	ZiboJson::Error = PyErr_NewException("zibo.json.JsonError", NULL, NULL);
	Py_INCREF(ZiboJson::Error);

	ZiboJson::DecodeError = PyErr_NewException("zibo.json.JsonDecodeError", ZiboJson::Error, NULL);
	Py_INCREF(ZiboJson::DecodeError);

	ZiboJson::EncodeError = PyErr_NewException("zibo.json.JsonEncodeError", ZiboJson::Error, NULL);
	Py_INCREF(ZiboJson::EncodeError);

	ZiboJson::TO_JSON_DEFAULT_METHOD_NAME = PyUnicode_InternFromString("__json__");
	ZiboJson::TZINFO_NAME = PyUnicode_InternFromString("tzinfo");
	ZiboJson::UTCOFFSET_METHOD_NAME = PyUnicode_InternFromString("utcoffset");
	ZiboJson::WRITE_METHOD_NAME = PyUnicode_InternFromString("write");

	PyModule_AddObject(module, "JsonError", ZiboJson::Error);
	PyModule_AddObject(module, "JsonDecodeError", ZiboJson::DecodeError);
	PyModule_AddObject(module, "JsonEncodeError", ZiboJson::EncodeError);

	Py_INCREF(&ZiboJson::HomogeneList_Type);
    PyModule_AddObject(module, "HomogeneList", (PyObject*)&ZiboJson::HomogeneList_Type);

	return module;
}