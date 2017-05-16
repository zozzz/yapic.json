#ifndef B7EE312A_D133_C640_1522_C4EFC8BF6F71
#define B7EE312A_D133_C640_1522_C4EFC8BF6F71

#include "config.h"
#include "util.h"
#include "pystring.h"
#include "globals.h"


#define ZIBO_JSON_BUFFER_INTERNAL_MAXCHAR 0


#if 0 && ZIBO_JSON_USE_SSE
#	include "sse.h"
#	define ZiboJson_Buffer_ConvertBytes(from_type, to_type, begin, l, to) \
		sse::string_copy<to_type, from_type>(to, begin, l * sizeof(from_type))
#else
#	define ZiboJson_Buffer_ConvertBytes(from_type, to_type, begin, l, to) \
		register to_type* data = to; \
		register from_type* buff = begin; \
		while (l--) { data[l] = buff[l]; }
#endif


namespace ZiboJson {

#define ZiboJson_Buffer_ASCII_MAXCHAR 	0x7F
#define ZiboJson_Buffer_EXTASCII_FROM	ZiboJson_Buffer_ASCII_MAXCHAR
#define ZiboJson_Buffer_1B_MAXCHAR 		0xFF
#define ZiboJson_Buffer_2B_FROM 		ZiboJson_Buffer_1B_MAXCHAR
#define ZiboJson_Buffer_2B_MAXCHAR 		0xFFFF
#define ZiboJson_Buffer_4B_FROM 		ZiboJson_Buffer_2B_MAXCHAR
#define ZiboJson_Buffer_4B_MAXCHAR 		0x10FFFF





// template<typename UCS>
// inline UCS DetermineMaxChar(register UCS* begin, register UCS* end) {
// 	register UCS maxchar = ZiboJson_Buffer_ASCII_MAXCHAR;
// 	while (begin < end) {
// 		if (*begin > maxchar) {
// 			maxchar = *begin;
// 		}
// 		++begin;
// 	}
// 	return maxchar;
// }


// template<>
// inline Py_UCS1 DetermineMaxChar(register Py_UCS1* begin, register Py_UCS1* end) {
// 	while (begin < end) {
// 		if (*(begin++) > ZiboJson_Buffer_EXTASCII_FROM) {
// 			return ZiboJson_Buffer_1B_MAXCHAR;
// 		}
// 	}
// 	return ZiboJson_Buffer_ASCII_MAXCHAR;
// }


// template<>
// inline Py_UCS2 DetermineMaxChar(register Py_UCS2* begin, register Py_UCS2* end) {
// 	register Py_UCS2 maxchar = ZiboJson_Buffer_ASCII_MAXCHAR;
// 	while(begin < end) {
// 		if (*begin > ZiboJson_Buffer_2B_FROM) {
// 			return ZiboJson_Buffer_2B_MAXCHAR;
// 		} else if (*begin > maxchar) {
// 			maxchar = *begin;
// 		}
// 		++begin;
// 	};
// 	return maxchar;
// }


// template<>
// inline Py_UCS4 DetermineMaxChar(Py_UCS4* begin, Py_UCS4* end) {
// 	register Py_UCS4 maxchar = ZiboJson_Buffer_ASCII_MAXCHAR;
// 	while(begin < end) {
// 		if (*begin > ZiboJson_Buffer_4B_FROM) {
// 			return ZiboJson_Buffer_4B_MAXCHAR;
// 		} else if (*begin > maxchar) {
// 			maxchar = *begin;
// 		}
// 		++begin;
// 	};
// 	return maxchar;
// }



template<typename T, size_t length>
class MemoryBuffer {
	public:
		typedef T Char;

		T initial[length];
		T* start;
		T* end;
		T* cursor;
#if !ZIBO_JSON_BUFFER_INTERNAL_MAXCHAR
		T maxchar;
#endif
		bool is_heap;

		inline explicit MemoryBuffer()
			: start(initial), end(start + length), cursor(initial),
#if !ZIBO_JSON_BUFFER_INTERNAL_MAXCHAR
			  maxchar(127),
#endif
			  is_heap(false) {
		}

		inline ~MemoryBuffer() {
			if (is_heap == true) {
				ZiboJson_Free(start);
			}
		}

		bool EnsureCapacity(Py_ssize_t required) {
			register Py_ssize_t current_usage = cursor - start;
			register Py_ssize_t new_size = end - start;
			required += current_usage;

#ifdef NDEBUG
			do {
				new_size <<= 1;
			} while (required > new_size);
#else
			new_size = required; // allocate only the required size, for testing
#endif

			if (is_heap == true) {
				start = (T*) ZiboJson_Realloc(start, sizeof(T) * new_size);
				if (start == NULL) {
					PyErr_NoMemory();
					return false;
				}
			} else {
				T* current_start = start;
				start = (T*) ZiboJson_Malloc(sizeof(T) * new_size);
				if (start == NULL) {
					PyErr_NoMemory();
					return false;
				}
				memmove(start, current_start,
					sizeof(T) == 1 ? current_usage :
					sizeof(T) == 2 ? current_usage << 1 :
					sizeof(T) == 4 ? current_usage << 2 :
					sizeof(T) * current_usage);
				is_heap = true;
			}

			cursor = start + current_usage;
			end = start + new_size;
			return true;
		}

#if 0

		inline PyObject* NewString() {
			Py_ssize_t l = cursor - start;
			cursor = start;
			return PyUnicode_FromKindAndData(PyUnicode_Traits<T>::Kind, start, l);
		}

#elif 1

	#if !ZIBO_JSON_BUFFER_INTERNAL_MAXCHAR

		PyObject* NewString() {
			return NewString(maxchar);
		}

		inline PyObject* NewString(T maxchar) {
	#else
		inline PyObject* NewString() {
	#endif
			// printf("TOTAL: %ld UNUSED: %ld\n", end-start, end - cursor);
			// printf("maxchar (%d) = %ld == %ld\n", sizeof(T), DetermineMaxChar(start, cursor), maxchar);
			register Py_ssize_t l = cursor - start;
			#if ZIBO_JSON_BUFFER_INTERNAL_MAXCHAR
				PyObject* str = PyUnicode_New(l, PyUnicode_Traits<T>::DetermineMaxChar(start, l));
			#else
				PyObject* str = PyUnicode_New(l, maxchar);
			#endif
			if (str == NULL) {
				return NULL;
			}

			switch (PyUnicode_KIND(str)) {
				case PyUnicode_1BYTE_KIND:
					if (sizeof(T) == 1) {
						memmove(PyUnicode_DATA(str), start, l);
					} else {
						ZiboJson_Buffer_ConvertBytes(T, Py_UCS1, start, l, PyUnicode_1BYTE_DATA(str));
					}
				break;

				case PyUnicode_2BYTE_KIND:
					if (sizeof(T) == 2) {
						memmove(PyUnicode_DATA(str), start, l << 1);
					} else {
						ZiboJson_Buffer_ConvertBytes(T, Py_UCS2, start, l, PyUnicode_2BYTE_DATA(str));
					}
				break;

				case PyUnicode_4BYTE_KIND:
					if (sizeof(T) == 4) {
						memmove(PyUnicode_DATA(str), start, l << 2);
					} else {
						ZiboJson_Buffer_ConvertBytes(T, Py_UCS4, start, l, PyUnicode_4BYTE_DATA(str));
					}
				break;
			}
			cursor = start;
			// maxchar = 127;
			return str;
		}
#endif
};








// template<typename T>
// class MemoryBuffer {
// 	public:
// 		typedef T Char;

// 		T* start;
// 		T* end;
// 		T* cursor;
// 		PyObject* string;
// 		T maxchar;
// 		T last_maxchar;
// 		bool is_heap;

// 		inline explicit MemoryBuffer(T* initial)
// 			: start(initial), end(start + ZIBO_JSON_ENCODER_BUFFER_SIZE), cursor(initial), string(NULL),
// 			  maxchar(127), last_maxchar(127), is_heap(false) {
// 		}

// 		inline ~MemoryBuffer() {
// 			if (string != NULL) {
// 				Py_CLEAR(string);
// 			} else if (is_heap == true) {
// 				ZiboJson_Free(start);
// 			}
// 		}

// 		bool EnsureCapacity(Py_ssize_t required) {
// 			register Py_ssize_t current_usage = cursor - start;
// 			register Py_ssize_t new_size = end - start;
// 			required += current_usage;

// #ifdef NDEBUG
// 			do {
// 				new_size <<= 1;
// 			} while (required > new_size);
// #else
// 			new_size = required; // allocate only the required size, for testing
// #endif

// 			if (sizeof(T) == 1) {
// 				if (is_heap == true) {
// 					if (PyUnicode_Resize(&string, new_size) != 0) {
// 						assert(0);
// 						return false;
// 					}
// 				} else {
// 					string = NewString();
// 					if (string == NULL) {
// 						assert(0);
// 						return false;
// 					}
// 					is_heap = true;
// 				}
// 				start = (T*) PyUnicode_1BYTE_DATA(string);
// 			} else {
// 				if (is_heap == true) {
// 					start = (T*) ZiboJson_Realloc(start, sizeof(T) * new_size);
// 					if (start == NULL) {
// 						PyErr_NoMemory();
// 						return false;
// 					}
// 				} else {
// 					T* current_start = start;
// 					start = (T*) ZiboJson_Malloc(sizeof(T) * new_size);
// 					if (start == NULL) {
// 						PyErr_NoMemory();
// 						return false;
// 					}
// 					memcpy(start, current_start,
// 						sizeof(T) == 1 ? current_usage :
// 						sizeof(T) == 2 ? current_usage << 1 :
// 						sizeof(T) == 4 ? current_usage << 2 :
// 						sizeof(T) * current_usage);
// 					is_heap = true;
// 				}
// 			}

// 			cursor = start + current_usage;
// 			end = start + new_size;
// 			return true;
// 		}

// 		inline PyObject* NewString() {
// 			printf("NewString is_heap=%i maxchar=%i last_maxchar=%i\n", is_heap, maxchar, last_maxchar);
// 			// printf("UNUSED: %ld\n", end - cursor);
// 			register Py_ssize_t l = cursor - start;

// 			if (sizeof(T) == 1) {
// 				if (is_heap == true && maxchar == last_maxchar) {
// 					printf("GGGG: %s\n", PyUnicode_1BYTE_DATA(string));
// 					if (PyUnicode_Resize(&string, l) == 0) {
// 						printf("CCCC: %s\n", PyUnicode_1BYTE_DATA(string));

// 						// assert(0);
// 						Py_INCREF(string);
// 						return string;
// 					} else {
// 						return NULL;
// 					}
// 				} else {
// 					PyObject* str = PyUnicode_New(l, maxchar);
// 					if (str == NULL) {
// 						return NULL;
// 					}
// 					assert(PyUnicode_KIND(str) == PyUnicode_1BYTE_KIND);
// 					memcpy(PyUnicode_DATA(str), start, l);
// 					last_maxchar = maxchar;
// 					return str;
// 				}
// 			} else {
// 				PyObject* str = PyUnicode_New(l, maxchar);
// 				if (str == NULL) {
// 					return NULL;
// 				}

// 				switch (PyUnicode_KIND(str)) {
// 					case PyUnicode_1BYTE_KIND:
// 						if (sizeof(T) == 1) {
// 							memcpy(PyUnicode_DATA(str), start, l);
// 						} else {
// 							#if ZIBO_JSON_USE_SSE
// 								sse::string_copy(PyUnicode_1BYTE_DATA(str), start, l);
// 							#else
// 								register Py_UCS1* data = PyUnicode_1BYTE_DATA(str);
// 								register T* buff = start;
// 								while (l--) { data[l] = buff[l]; }
// 							#endif
// 						}
// 					break;

// 					case PyUnicode_2BYTE_KIND:
// 						if (sizeof(T) == 2) {
// 							memcpy(PyUnicode_DATA(str), start, l << 1);
// 						} else {
// 							#if ZIBO_JSON_USE_SSE
// 								sse::string_copy(PyUnicode_2BYTE_DATA(str), start, l << 1);
// 							#else
// 								register Py_UCS2* data = PyUnicode_2BYTE_DATA(str);
// 								register T* buff = start;
// 								// TODO parallel pragma
// 								while (l--) { data[l] = buff[l]; }
// 							#endif
// 						}
// 					break;

// 					case PyUnicode_4BYTE_KIND:
// 						if (sizeof(T) == 4) {
// 							memcpy(PyUnicode_DATA(str), start, l << 2);
// 						} else {
// 							#if ZIBO_JSON_USE_SSE
// 								sse::string_copy(PyUnicode_4BYTE_DATA(str), start, l << 2);
// 							#else
// 								register Py_UCS4* data = PyUnicode_4BYTE_DATA(str);
// 								register T* buff = start;
// 								while (l--) { data[l] = buff[l]; }
// 							#endif
// 						}
// 					break;
// 				}
// 				return str;
// 			}
// 		}
// };











// template<typename T>
// class MemoryBuffer: public AbstractBuffer<MemoryBuffer, T> {
// 	public:
// 		inline explicit MemoryBuffer(T* initial)
// 			: AbstractBuffer(initial) {
// 		}
// };


template<typename T, size_t length>
class FileBuffer: public MemoryBuffer<T, length> {
	public:
		PyObject* write;
		// FILE fileno;

		using MemoryBuffer<T, length>::MemoryBuffer;

		inline ~FileBuffer() {
			Py_CLEAR(write);
		}

		inline bool SetTarget(PyObject* target) {
			write = PyObject_GetAttr(target, WRITE_METHOD_NAME);
			return write != NULL;
		}

		inline bool Flush() {
			assert(write);

			if (this->cursor != this->start) {
				PyObject* str = this->NewString();
				if (str == NULL) {
					return false;
				}

				PyObject* res = PyObject_CallFunctionObjArgs(write, str, NULL);
				Py_DECREF(str);
				if (res == NULL) {
					return false;
				}
				Py_DECREF(res);
				// this->cursor = this->start;
				// this->maxchar = 127;
			}
			return true;
		}

		bool EnsureCapacity(Py_ssize_t required) {
			if (EXPECT_TRUE(Flush())) {
				if (this->end - this->start < required) {
					return MemoryBuffer<T, length>::EnsureCapacity(required);
				} else {
					return true;
				}
			} else {
				return false;
			}
		}
};


#define ZiboJson_Buffer_IncMaxchar(buffer, ch) \
	if ((ch) > ZiboJson_Buffer_EXTASCII_FROM) { \
		if ((ch) > ZiboJson_Buffer_2B_FROM) { \
			if ((ch) > ZiboJson_Buffer_4B_FROM) { \
				(buffer).maxchar = ZiboJson_Buffer_4B_MAXCHAR; \
			} else { \
				(buffer).maxchar |= ZiboJson_Buffer_2B_MAXCHAR; \
			} \
		} else { \
			(buffer).maxchar |= ZiboJson_Buffer_1B_MAXCHAR; \
		} \
	}

#define ZiboJson_Buffer_HighestMaxChar(ch) \
	((ch) > ZiboJson_Buffer_4B_FROM ? ZiboJson_Buffer_4B_MAXCHAR : ZiboJson_Buffer_2B_MAXCHAR)

} /* end namespace ZiboJson */

#endif /* B7EE312A_D133_C640_1522_C4EFC8BF6F71 */
