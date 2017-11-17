#ifndef F1CD3B20_F133_C6A6_CB3D_69E6D2C02DAA
#define F1CD3B20_F133_C6A6_CB3D_69E6D2C02DAA

namespace Yapic { namespace Json {


typedef struct {
	PyObject_HEAD
} HomogeneList;


static PyObject* HomogeneList_Iter(PyObject* self) {
	PyObject* type = PyObject_Type(self);
	if (type == NULL) {
		return NULL;
	}
	PyObject* name = PyObject_GetAttrString(type, "__qualname__");
	if (name == NULL) {
		return NULL;
	}
	PyErr_Format(PyExc_NotImplementedError, "%U.__iter__", name);
	// PyErr_SetString(PyExc_NotImplementedError, "yapic.json.HomogeneList.__iter__");
	return NULL;
}


static PyTypeObject HomogeneList_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
    /* tp_name */ 			"yapic.json.HomogeneList",
    /* tp_basicsize */ 		sizeof(HomogeneList),
    /* tp_itemsize */ 		0,
    /* tp_dealloc */ 		0,
    /* tp_print */ 			0,
    /* tp_getattr */ 		0,
    /* tp_setattr */ 		0,
    /* tp_reserved */ 		0,
    /* tp_repr */ 			0,
    /* tp_as_number */ 		0,
    /* tp_as_sequence */ 	0,
    /* tp_as_mapping */ 	0,
    /* tp_hash  */ 			0,
    /* tp_call */ 			0,
    /* tp_str */ 			0,
    /* tp_getattro */ 		0,
    /* tp_setattro */ 		0,
    /* tp_as_buffer */ 		0,
    /* tp_flags */ 			Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IS_ABSTRACT,
    /* tp_doc */ 			0,
	/* tp_traverse */ 		0,
	/* tp_clear */ 			0,
	/* tp_richcompare */ 	0,
	/* tp_weaklistoffset */ 0,
	/* tp_iter */ 			(getiterfunc)HomogeneList_Iter,
	/* tp_iternext */ 		0,
	/* tp_methods */ 		0,
	/* tp_members */ 		0,
	/* tp_getset */ 		0,
	/* tp_base */ 			0,
	/* tp_dict */ 			0,
	/* tp_descr_get */ 		0,
	/* tp_descr_set */ 		0,
	/* tp_dictoffset */ 	0,
	/* tp_init */ 			0,
	/* tp_alloc */ 			0,
	/* tp_new */ 			PyType_GenericNew,
	/* tp_free */ 			0
};


} /* end namespace Json */
} /* end namespace Yapic */

#endif /* F1CD3B20_F133_C6A6_CB3D_69E6D2C02DAA */
