#ifndef B7EE312A_D133_C640_1522_C4EFC8BF6F71
#define B7EE312A_D133_C640_1522_C4EFC8BF6F71

#include "config.h"
#include "util.h"
#include "globals.h"


namespace ZiboJson {

#define ZiboJson_Buffer_ASCII_MAXCHAR 	0x7F
#define ZiboJson_Buffer_EXTASCII_FROM	ZiboJson_Buffer_ASCII_MAXCHAR
#define ZiboJson_Buffer_1B_MAXCHAR 		0xFF
#define ZiboJson_Buffer_2B_FROM 		ZiboJson_Buffer_1B_MAXCHAR
#define ZiboJson_Buffer_2B_MAXCHAR 		0xFFFF
#define ZiboJson_Buffer_4B_FROM 		ZiboJson_Buffer_2B_MAXCHAR
#define ZiboJson_Buffer_4B_MAXCHAR 		0x10FFFF


template<typename T, size_t length>
class MemoryBuffer {
	public:
		typedef T Char;

		T initial[length];
		T* start;
		T* end;
		T* cursor;
		T maxchar;
		bool is_heap;

		inline explicit MemoryBuffer()
			: start(initial), end(start + length), cursor(initial),
			  maxchar(127), is_heap(false) {
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
				CopyBytes(start, current_start, current_usage);
				is_heap = true;
			}

			cursor = start + current_usage;
			end = start + new_size;
			return true;
		}

		inline PyObject* NewString() {
			Py_ssize_t l = cursor - start;
			PyObject* str = PyUnicode_New(l, maxchar);
			if (str != NULL) {
				switch (PyUnicode_KIND(str)) {
					case PyUnicode_1BYTE_KIND:
						CopyBytes(PyUnicode_1BYTE_DATA(str), start, l);
					break;

					case PyUnicode_2BYTE_KIND:
						CopyBytes(PyUnicode_2BYTE_DATA(str), start, l);
					break;

					case PyUnicode_4BYTE_KIND:
						CopyBytes(PyUnicode_4BYTE_DATA(str), start, l);
					break;
				}
			}
			return str;
		}
};



template<typename T, size_t length>
class FileBuffer: public MemoryBuffer<T, length> {
	public:
		typedef MemoryBuffer<T, length> Base;

		PyObject* write;
		// FILE fileno;

		using Base::MemoryBuffer;

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
				this->cursor = this->start;
				this->maxchar = 127;
			}
			return true;
		}

		bool EnsureCapacity(Py_ssize_t required) {
			if (EXPECT_TRUE(Flush())) {
				if (this->end - this->start < required) {
					return Base::EnsureCapacity(required);
				} else {
					return true;
				}
			} else {
				return false;
			}
		}
};



enum ChunkKind {
	Chunk_1BYTE_KIND,
	Chunk_2BYTE_KIND,
	Chunk_4BYTE_KIND,
	Chunk_CHAR_KIND
};


template<typename T>
struct ChunkKindByType;

template<>
struct ChunkKindByType<Py_UCS1> { static const ChunkKind Kind = Chunk_1BYTE_KIND; };

template<>
struct ChunkKindByType<Py_UCS2> { static const ChunkKind Kind = Chunk_2BYTE_KIND; };

template<>
struct ChunkKindByType<Py_UCS4> { static const ChunkKind Kind = Chunk_4BYTE_KIND; };


class ChunkBuffer {
	public:
		struct Chunk {
			void* data;
			Py_ssize_t length;
			ChunkKind kind;
		};

		Chunk initial[ZIBO_JSON_CHUNK_BUFFER_SIZE];
		Chunk* chunksBegin;
		Chunk* chunksEnd;
		Chunk* chunk;

		Py_ssize_t totalLength;

		inline ChunkBuffer()
			: chunksBegin(initial), chunksEnd(initial + ZIBO_JSON_CHUNK_BUFFER_SIZE), chunk(initial),
			  totalLength(0) {

		}

		inline ~ChunkBuffer() {
			if (initial != chunksBegin) {
				ZiboJson_Free(chunksBegin);
			}
		}

		template<typename T>
		inline void StartSlice(T* start) {
			chunk->data = start;
			chunk->kind = ChunkKindByType<T>::Kind;
		}

		template<typename T>
		inline bool CloseSlice(T* end) {
			assert(chunk->kind == ChunkKindByType<T>::Kind);
			totalLength += (chunk->length = end - (T*)(chunk->data));
			return GotoNextChunk();
		}

		template<typename T>
		inline bool AppendChar(T ch) {
			chunk->length = ch;
			chunk->kind = Chunk_CHAR_KIND;
			++totalLength;
			return GotoNextChunk();
		}

		PyObject* NewString(Py_ssize_t maxchar) {
			PyObject* str = PyUnicode_New(totalLength, maxchar);
			if (str != NULL) {
				switch (PyUnicode_KIND(str)) {
					case PyUnicode_1BYTE_KIND:
						Write(PyUnicode_1BYTE_DATA(str));
					break;

					case PyUnicode_2BYTE_KIND:
						Write(PyUnicode_2BYTE_DATA(str));
					break;

					default:
						assert(PyUnicode_KIND(str) == PyUnicode_4BYTE_KIND);
						Write(PyUnicode_4BYTE_DATA(str));
					break;
				}
			}
			return str;
		}

		template<typename O>
		inline void Write(O* target) const {
			register Chunk* c = chunksBegin;

			while (c++ < chunk) {
				switch (c->kind) {
					case Chunk_1BYTE_KIND:
						CopyBytes(target, (Py_UCS1*) c->data, c->length);
						target += c->length;
					break;

					case Chunk_2BYTE_KIND:
						CopyBytes(target, (Py_UCS2*) c->data, c->length);
						target += c->length;
					break;

					case Chunk_4BYTE_KIND:
						CopyBytes(target, (Py_UCS4*) c->data, c->length);
						target += c->length;
					break;

					case Chunk_CHAR_KIND:
						*(target++) = (O) c->length;
					break;
				}
			}
		}

		inline void Reset() {
			chunk = chunksBegin;
			totalLength = 0;
		}

	private:
		inline bool GotoNextChunk() {
			return (++chunk >= chunksEnd ? Resize() : true);
		}

		bool Resize() {
			size_t consumed = chunk - chunksBegin;
			size_t l = (chunksEnd - chunksBegin) << 1;
			if (initial == chunksBegin) {
				chunksBegin = (Chunk*) ZiboJson_Malloc(sizeof(Chunk) * l);
				if (chunksBegin == NULL) {
					PyErr_NoMemory();
					return NULL;
				}
				memmove(chunksBegin, initial, sizeof(Chunk) * l);
			} else {
				chunksBegin = (Chunk*) ZiboJson_Realloc(chunksBegin, sizeof(Chunk) * l);
				if (chunksBegin == NULL) {
					PyErr_NoMemory();
					return NULL;
				}
			}

			chunk = chunksBegin + consumed;
			chunksEnd = chunksBegin + l;
			return true;
		}
};



} /* end namespace ZiboJson */

#endif /* B7EE312A_D133_C640_1522_C4EFC8BF6F71 */
