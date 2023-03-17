#ifndef G627F265_3133_C70A_1248_E27DABF48BD8
#define G627F265_3133_C70A_1248_E27DABF48BD8

#include <yapic/module.hpp>
#include <yapic/pyptr.hpp>
#include "config.h"

namespace Yapic { namespace Json {

class Module: public Yapic::Module<Module> {
public:
	static constexpr const char* __name__ = "yapic.json";

	using ModuleVar = Yapic::ModuleVar<Module>;
	using ModuleExc = Yapic::ModuleExc<Module>;
	using ModuleRef = Yapic::ModuleRef<Module>;

	ModuleRef ItemsView;
	ModuleRef Decimal;
	ModuleRef UUID;
	ModuleRef Enum;
	ModuleRef json;
	ModuleVar JSONDecodeError;
	ModuleRef PyTimezone;
	ModuleVar PyUTCTimezone;
	ModuleRef Dataclass_FIELDS;
	ModuleRef Dataclass_Field;

	ModuleVar STR_TZINFO;
	ModuleVar STR_UTCOFFSET;
	ModuleVar STR_WRITE;
	ModuleVar STR_TOJSON;
	ModuleVar STR_VALUE;
	ModuleVar __version__;

	ModuleExc Error;
	ModuleExc EncodeError;
	ModuleExc DecodeError;

	static inline int __init__(PyObject* module, Module* state) {
		PyDateTime_IMPORT;

		state->ItemsView.Import("collections.abc", "ItemsView");
		state->Decimal.Import("decimal", "Decimal");
		state->UUID.Import("uuid", "UUID");
		state->Enum.Import("enum", "Enum");
		state->json.Import("json");
		state->JSONDecodeError = PyObject_GetAttrString(state->json, "JSONDecodeError");
		state->PyTimezone.Import("datetime", "timezone");
		state->PyUTCTimezone = PyObject_GetAttrString(state->PyTimezone, "utc");
		state->Dataclass_FIELDS.Import("dataclasses", "_FIELDS");
		state->Dataclass_Field.Import("dataclasses", "Field");

		state->STR_TZINFO = "tzinfo";
		state->STR_UTCOFFSET = "utcoffset";
		state->STR_WRITE = "write";
		state->STR_TOJSON = "__json__";
		state->STR_VALUE = "value";
		state->__version__.Value(YAPIC_JSON_VERSION_STR).Export("__version__");

		state->Error.Define("JsonError", PyExc_ValueError);

		PyPtr<PyTupleObject> bases = PyTuple_New(2);
		if (bases.IsNull()) {
			throw ModuleExc::Error;
		}
		Py_INCREF(state->Error);
		Py_INCREF(state->JSONDecodeError);
		PyTuple_SET_ITEM(bases, 0, state->Error);
		PyTuple_SET_ITEM(bases, 1, state->JSONDecodeError);
		state->DecodeError.Define("JsonDecodeError", bases);
		state->DecodeError.Export("JSONDecodeError");

		state->EncodeError.Define("JsonEncodeError", state->Error);
		return 0;
	}

	static PyObject *dumps(PyObject *module, PyObject *args, PyObject *kwargs);
	static PyObject *dumpb(PyObject *module, PyObject *args, PyObject *kwargs);
	static PyObject *dump(PyObject *module, PyObject *args, PyObject *kwargs);
	static PyObject *loads(PyObject *module, PyObject *args, PyObject *kwargs);
	// static PyObject *load(PyObject *self, PyObject *args, PyObject *kwargs);

	Yapic_METHODS_BEGIN
		Yapic_Method(dumps, METH_VARARGS | METH_KEYWORDS, NULL)
		Yapic_Method(dumpb, METH_VARARGS | METH_KEYWORDS, NULL)
		Yapic_Method(dump, METH_VARARGS | METH_KEYWORDS, NULL)
		Yapic_Method(loads, METH_VARARGS | METH_KEYWORDS, NULL)
	Yapic_METHODS_END
};


} /* end namespace Json */
} /* end namespace Yapic */

#endif /* G627F265_3133_C70A_1248_E27DABF48BD8 */
