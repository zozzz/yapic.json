#ifndef B7EE312A_D133_C640_1522_C4EFC8BF6F71
#define B7EE312A_D133_C640_1522_C4EFC8BF6F71

#include "config.h"
#include "util.h"
#include "json.h"


namespace Yapic { namespace Json {

#define YapicJson_Buffer_ASCII_MAXCHAR 	0x7F
#define YapicJson_Buffer_EXTASCII_FROM	YapicJson_Buffer_ASCII_MAXCHAR
#define YapicJson_Buffer_1B_MAXCHAR 		0xFF
#define YapicJson_Buffer_2B_FROM 		YapicJson_Buffer_1B_MAXCHAR
#define YapicJson_Buffer_2B_MAXCHAR 		0xFFFF
#define YapicJson_Buffer_4B_FROM 		YapicJson_Buffer_2B_MAXCHAR
#define YapicJson_Buffer_4B_MAXCHAR 		0x10FFFF

#define YapicJson_Buffer_Kind_FromChar(ch) \
	((uint8_t)((ch) < YapicJson_Buffer_1B_MAXCHAR \
		? PyUnicode_1BYTE_KIND \
		: (ch) < YapicJson_Buffer_2B_MAXCHAR \
			? PyUnicode_2BYTE_KIND \
			: PyUnicode_4BYTE_KIND))

#define YapicJson_Buffer_Kind_ToChar(kind) \
	((kind) == PyUnicode_1BYTE_KIND  \
		? YapicJson_Buffer_1B_MAXCHAR - 1 \
		: (kind) == PyUnicode_2BYTE_KIND \
			? YapicJson_Buffer_2B_MAXCHAR - 1 \
			: YapicJson_Buffer_4B_MAXCHAR - 1)


#define MemoryBuffer_HasEnoughCapacity(__buffer, __required) \
	((__required) < (__buffer).end - (__buffer).cursor)

#define MemoryBuffer_EnsureCapacity(__buffer, __required) \
	(LIKELY(MemoryBuffer_HasEnoughCapacity(__buffer, __required) || (__buffer).EnsureCapacity(__required)))


template<typename T, Py_ssize_t SIZE>
class MemoryBuffer {
	public:
		typedef T Char;
		static constexpr Py_ssize_t InitialSize = SIZE;

		T* cursor;
		T* start;
		T* end;
		T maxchar;
		bool is_heap;
		T initial[SIZE];

		inline explicit MemoryBuffer()
			: cursor(initial), start(initial), end(start + SIZE),
			  maxchar(127), is_heap(false) {
		}

		inline ~MemoryBuffer() {
			if (is_heap == true) {
				YapicJson_Free(start);
			}
		}

		bool EnsureCapacity(Py_ssize_t required) {
			Py_ssize_t current_usage = cursor - start;
			Py_ssize_t new_size = end - start;
			required += current_usage;

#ifdef NDEBUG
			do {
				new_size <<= 1;
			} while (required > new_size);
#else
			new_size = required; // allocate only the required size, for testing
#endif

			if (is_heap == true) {
				start = (T*) YapicJson_Realloc(start, sizeof(T) * new_size);
				IF_UNLIKELY (start == NULL) {
					PyErr_NoMemory();
					return false;
				}
			} else {
				start = (T*) YapicJson_Malloc(sizeof(T) * new_size);
				IF_UNLIKELY (start == NULL) {
					PyErr_NoMemory();
					return false;
				}
				CopyBytes(start, initial, current_usage);
				is_heap = true;
			}

			cursor = start + current_usage;
			end = start + new_size;
			return true;
		}

		inline PyObject* NewString() {
			return NewString(maxchar);
		}

		PyObject* NewString(T maxchar) {
			Py_ssize_t l = cursor - start;
			PyObject* str = PyUnicode_New(l, maxchar);
			IF_LIKELY (str != NULL) {
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

		inline PyObject* NewBytes() {
			assert(sizeof(T) == 1);
			return PyBytes_FromStringAndSize(reinterpret_cast<char*>(start), cursor - start);
		}

		inline void Reset() {
			cursor = start;
			maxchar = 127;
		}

		bool AppendChar(T ch) {
			IF_LIKELY (1 < end - cursor || EnsureCapacity(1)) {
				*(cursor++) = ch;
				return true;
			} else {
				return false;
			}
		}

		template<typename CT>
		bool AppendSlice(CT *data, Py_ssize_t size) {
			IF_LIKELY (size < end - cursor || EnsureCapacity(size)) {
				CopyBytes(cursor, data, size);
				cursor += size;
				return true;
			} else {
				return false;
			}
		}
};


template<typename T, Py_ssize_t SIZE>
class BytesBuffer {
	public:
		typedef T Char;
		static constexpr Py_ssize_t InitialSize = SIZE;

		T* start;
		T* end;
		T* cursor;
		PyObject* bytes;
		T maxchar;
		T initial[SIZE];

		inline explicit BytesBuffer()
			: start(initial), end(start + SIZE), cursor(initial), bytes(NULL), maxchar(127) {
		}

		inline ~BytesBuffer() {
			Py_CLEAR(bytes);
		}

		bool EnsureCapacity(Py_ssize_t required) {
			Py_ssize_t current_usage = cursor - start;
			Py_ssize_t new_size = end - start;
			required += current_usage;

#ifdef NDEBUG
			do {
				new_size <<= 1;
			} while (required > new_size);
#else
			new_size = required; // allocate only the required size, for testing
#endif

			if (bytes != NULL) {
				if (_PyBytes_Resize(&bytes, new_size) == -1) {
					return false;
				}
				start = reinterpret_cast<T*>(PyBytes_AS_STRING(bytes));
			} else {
				bytes = PyBytes_FromStringAndSize(NULL, new_size);
				if (bytes == NULL) {
					return false;
				}
				start = reinterpret_cast<T*>(PyBytes_AS_STRING(bytes));
				CopyBytes(start, initial, current_usage);
			}

			cursor = start + current_usage;
			end = start + new_size;
			return true;
		}

		inline PyObject* NewString() {
			return NewString(0);
		}

		PyObject* NewString(T maxchar) {
			if (bytes != NULL) {
				if (_PyBytes_Resize(&bytes, cursor - start) == 0) {
					PyObject* tmp = bytes;
					bytes = NULL;
					return tmp;
				} else {
					return NULL;
				}
			} else {
				return PyBytes_FromStringAndSize((char*) initial, cursor - start);
			}
		}

		inline void Reset() {
			Py_CLEAR(bytes);
			start = initial;
			end = start + SIZE;
			cursor = start;
		}

		bool AppendChar(T ch) {
			IF_LIKELY (1 < end - cursor || EnsureCapacity(1)) {
				*(cursor++) = ch;
				return true;
			} else {
				return false;
			}
		}

		template<typename CT>
		bool AppendSlice(CT *data, Py_ssize_t size) {
			IF_LIKELY (size < end - cursor || EnsureCapacity(size)) {
				CopyBytes(cursor, data, size);
				cursor += size;
				return true;
			} else {
				return false;
			}
		}
};



template<typename T, size_t length>
class FileBuffer: public MemoryBuffer<T, length> {
	public:
		typedef MemoryBuffer<T, length> Base;

		PyObject* write;
		// FILE fileno;

		using Base::Base;

		inline ~FileBuffer() {
			Py_CLEAR(write);
		}

		inline bool SetTarget(PyObject* target) {
			write = PyObject_GetAttr(target, Module::State()->STR_WRITE);
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
			IF_LIKELY (Flush()) {
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

		Chunk initial[YAPIC_JSON_CHUNK_BUFFER_SIZE];
		Chunk* chunksBegin;
		Chunk* chunksEnd;
		Chunk* chunk;

		Py_ssize_t totalLength;

		inline ChunkBuffer()
			: chunksBegin(initial), chunksEnd(initial + YAPIC_JSON_CHUNK_BUFFER_SIZE), chunk(initial),
			  totalLength(0) {

		}

		inline ~ChunkBuffer() {
			if (initial != chunksBegin) {
				YapicJson_Free(chunksBegin);
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
		inline bool AppendSlice(T* data, Py_ssize_t size) {
			chunk->data = data;
			chunk->kind = ChunkKindByType<T>::Kind;
			totalLength += (chunk->length = size);
			return GotoNextChunk();
		}

		template<typename T>
		inline bool AppendChar(T ch) {
			chunk->length = ch;
			chunk->kind = Chunk_CHAR_KIND;
			++totalLength;
			return GotoNextChunk();
		}

		PyObject* NewString(Py_UCS4 maxchar) {
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
			Chunk* c = chunksBegin;

			for (; c < chunk ; c++) {
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

					#if !NDEBUG
					default:
						assert(0);
					break;
					#endif
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
				chunksBegin = (Chunk*) YapicJson_Malloc(sizeof(Chunk) * l);
				if (chunksBegin == NULL) {
					PyErr_NoMemory();
					return false;
				}
				memmove(chunksBegin, initial, sizeof(Chunk) * consumed);
			} else {
				chunksBegin = (Chunk*) YapicJson_Realloc(chunksBegin, sizeof(Chunk) * l);
				if (chunksBegin == NULL) {
					PyErr_NoMemory();
					return false;
				}
			}

			chunk = chunksBegin + consumed;
			chunksEnd = chunksBegin + l;
			return true;
		}
};


} /* end namespace Json */
} /* end namespace Yapic */

#endif /* B7EE312A_D133_C640_1522_C4EFC8BF6F71 */
