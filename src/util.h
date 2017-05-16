#ifndef U492A975_4133_C6A2_1678_FF0AE59B4CBD
#define U492A975_4133_C6A2_1678_FF0AE59B4CBD

#include "config.h"

#define ZiboJson_UnrolledLoopEnd(length, loopSize) ((length) & ~((loopSize) - 1))

namespace ZiboJson {

/*
static inline PyObject* CallPyFunction1Arg(PyObject* callable, PyObject* arg) {
	PyObject* args = PyTuple_New(1);
	Py_INCREF(arg);
	PyTuple_SET_ITEM(args, 0, arg);
	PyObject* res = PyObject_Call(callable, args, NULL);

	assert(((PyObject *)args)->ob_refcnt == 1);
	assert(((PyObject *)arg)->ob_refcnt > 1);

	Py_DECREF(args);

	return res;
}
*/

} /* end namespace ZiboJson */

#endif /* U492A975_4133_C6A2_1678_FF0AE59B4CBD */
