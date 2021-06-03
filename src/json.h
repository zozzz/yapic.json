#ifndef G627F265_3133_C70A_1248_E27DABF48BD8
#define G627F265_3133_C70A_1248_E27DABF48BD8

#include <yapic/module.hpp>
#include "config.h"

namespace Yapic { namespace Json {

class Module: public Yapic::Module<Module> {
public:
	static constexpr const char* __name__ = "yapic.json";

	using ModuleVar = Yapic::ModuleVar<Module>;
	using ModuleExc = Yapic::ModuleExc<Module>;
	using ModuleRef = Yapic::ModuleRef<Module>;

	ModuleRef PyTimezone;
	ModuleRef ItemsView;
	ModuleRef Decimal;
	ModuleRef UUID;
	ModuleRef Enum;
	ModuleVar PyUTCTimezone;

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

		state->PyTimezone.Import("datetime", "timezone");
		state->ItemsView.Import("collections.abc", "ItemsView");
		state->Decimal.Import("decimal", "Decimal");
		state->UUID.Import("uuid", "UUID");
		state->Enum.Import("enum", "Enum");
		state->PyUTCTimezone = PyObject_GetAttrString(state->PyTimezone, "utc");

		state->STR_TZINFO = "tzinfo";
		state->STR_UTCOFFSET = "utcoffset";
		state->STR_WRITE = "write";
		state->STR_TOJSON = "__json__";
		state->STR_VALUE = "value";
		state->__version__.Value(YAPIC_JSON_VERSION_STR).Export("__version__");

		state->Error.Define("JsonError");
		state->EncodeError.Define("JsonEncodeError", state->Error);
		state->DecodeError.Define("JsonDecodeError", state->Error);
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
