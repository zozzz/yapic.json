#ifndef FA06F563_6133_C709_128F_2D11FE8957D4
#define FA06F563_6133_C709_128F_2D11FE8957D4

#include "../libs/double-conversion/double-conversion/utils.h"
#include "config.h"

namespace ZiboJson {

// #define Chunk_1BYTE_KIND 0
// #define Chunk_2BYTE_KIND 1
// #define Chunk_4BYTE_KIND 2
// #define Chunk_CHAR_KIND  3

#define ChunkBuffer_GetKindByType(T) \
	(assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4), \
	(sizeof(T) == 1 ? Chunk_1BYTE_KIND : \
	 sizeof(T) == 2 ? Chunk_2BYTE_KIND : \
	 /*sizeof(T) == 4*/ Chunk_4BYTE_KIND))


#if 0


template<typename T, size_t L>
class Array {
	public:
		inline explicit Array()
			: data(initial), length(L), cursor(0) {
		}

		inline T& operator[] (int idx) {
			return data[idx];
		}

		inline T& Current() {
			return data[cursor];
		}

		inline bool Inc() {
			if (length <= ++cursor) {
				return Resize(length << 1);
			}
			return true;
		}

		inline size_t Position() {
			return cursor;
		}

		inline void Reset() {
			cursor = 0;
		}

		bool Resize(size_t newLength) {
			if (initial != data) {
				data = (T*) ZiboJson_Realloc(data, newLength * sizeof(T));
				if (data == NULL) {
					PyErr_NoMemory();
					return false;
				}
			} else {
				T* heap = (T*) ZiboJson_Malloc(newLength * sizeof(T));
				if (heap == NULL) {
					PyErr_NoMemory();
					return false;
				}
				memmove(heap, data, length * sizeof(T));
				data = heap;
			}
			length = newLength;
			return true;
		}

		inline ~Array() {
			if (initial != data) {
				ZiboJson_Free(data);
			}
		}

	private:
		T initial[L];
		T* data;
		size_t length;
		size_t cursor;
};


// template<typename CHAR_T>
class ChunkBuffer {
	public:
		enum ChunkKind {
			Chunk_1BYTE_KIND,
			Chunk_2BYTE_KIND,
			Chunk_4BYTE_KIND,
			Chunk_CHAR_KIND
		};

		struct Chunk {
			void* data;
			Py_ssize_t length;
			// Py_ssize_t kind; // align miatt lett Py_ssize_t
			ChunkKind kind;

			// disable copy
			// inline Chunk() {};
			// Chunk(const Chunk&) = delete;
			// Chunk& operator=(Chunk const&) = delete;
		};

		// Array<CHAR_T, 1024> chars;
		Array<Chunk, 256> chunks;

		Py_ssize_t totalLength;

		inline ChunkBuffer()
			: totalLength(0) {

		}

		template<typename T>
		inline void StartSlice(T* start) {
			Chunk& chunk = chunks.Current();
			chunk.data = start;
			chunk.kind = ChunkBuffer_GetKindByType(T);
			chunk.length = 0;
		}

		template<typename T>
		inline bool CloseSlice(T* end) {
			Chunk& chunk = chunks.Current();

			assert(chunk.kind == ChunkBuffer_GetKindByType(T));

			chunk.length = end - (T*)(chunk.data);
			totalLength += chunk.length;
			return chunks.Inc();
		}

		// template<typename T>
		// inline bool AppendChar(T ch) {
		// 	assert(sizeof(T) <= sizeof(CHAR_T));

		// 	chars.Current() = ch;
		// 	Chunk& chunk = chunks.Current();

		// 	chunk.data = NULL;
		// 	chunk.length = chars.Position();
		// 	chunk.kind = ChunkBuffer_GetKindByType(T);
		// 	++totalLength;

		// 	return chars.Inc() && chunks.Inc();
		// }

		template<typename T>
		inline bool AppendChar(T ch) {
			// assert(sizeof(T) <= sizeof(CHAR_T));

			// chars.Current() = ch;
			Chunk& chunk = chunks.Current();
			chunk.data = NULL;
			chunk.length = ch;
			chunk.kind = Chunk_CHAR_KIND;
			++totalLength;

			return chunks.Inc();
		}

		PyObject* NewString(Py_ssize_t maxchar) {
			// printf("NewString(%ld) TL = %ld\n", maxchar, totalLength);
			PyObject* str = PyUnicode_New(totalLength, maxchar);
			if (str == NULL) {
				return str;
			}

			switch (PyUnicode_KIND(str)) {
				case PyUnicode_1BYTE_KIND:
					Write(PyUnicode_1BYTE_DATA(str));
				break;

				case PyUnicode_2BYTE_KIND:
					Write(PyUnicode_2BYTE_DATA(str));
				break;

				case PyUnicode_4BYTE_KIND:
					Write(PyUnicode_4BYTE_DATA(str));
				break;
			}

			// printf("NewString End\n");

			chunks.Reset();
			totalLength = 0;
			// chars.Reset();
			return str;
		}

		template<typename O>
		inline void Write(O* target) {
			for (size_t i = 0 ; i < chunks.Position() ; i++) {
				// printf("Write i=%ld\n", i);
				switch (chunks[i].kind) {
					case Chunk_1BYTE_KIND:
						WriteChunk<Py_UCS1, O>(chunks[i], target, &target);
					break;

					case Chunk_2BYTE_KIND:
						WriteChunk<Py_UCS2, O>(chunks[i], target, &target);
					break;

					case Chunk_4BYTE_KIND:
						WriteChunk<Py_UCS4, O>(chunks[i], target, &target);
					break;

					case Chunk_CHAR_KIND:
						*(target++) = (O) chunks[i].length;
					break;
				}
			}
			// printf("Write End\n");
		}

	private:
		template<typename I, typename O>
		inline void WriteChunk(Chunk& chunk, O* target, O** targetOut) {
			assert(sizeof(I) <= sizeof(O));

			if (sizeof(I) == sizeof(O)) {
				memmove(target, chunk.data, chunk.length * sizeof(I));
				*targetOut = target + chunk.length;
			} else {
				register I* data = (I*) chunk.data;
				register Py_ssize_t l = chunk.length;
				while (l-- > 0) {
					target[l] = data[l];
				}
				*targetOut = target + chunk.length;
			}
		}
};

#else

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


// template<typename CHAR_T>
class ChunkBuffer {
	public:
		struct Chunk {
			void* data;
			Py_ssize_t length;
			ChunkKind kind;
		};

		Chunk initial[256];
		Chunk* chunksBegin;
		Chunk* chunksEnd;
		Chunk* chunk;

		Py_ssize_t totalLength;

		inline ChunkBuffer()
			: chunksBegin(initial), chunksEnd(initial + 256), chunk(initial),
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
			if (str == NULL) {
				return str;
			}

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
			// chunk = chunksBegin;
			// totalLength = 0;
			return str;
		}

		template<typename O>
		inline void Write(O* target) const {
			register Chunk* c = chunksBegin;

			while (c < chunk) {
				switch (c->kind) {
					case Chunk_1BYTE_KIND:
						WriteChunk<Py_UCS1, O>(c, target, &target);
					break;

					case Chunk_2BYTE_KIND:
						WriteChunk<Py_UCS2, O>(c, target, &target);
					break;

					case Chunk_4BYTE_KIND:
						WriteChunk<Py_UCS4, O>(c, target, &target);
					break;

					case Chunk_CHAR_KIND:
						*(target++) = (O) c->length;
					break;
				}
				++c;
			}
		}

		inline void Reset() {
			chunk = chunksBegin;
			totalLength = 0;
		}

	private:
		template<typename I, typename O>
		inline void WriteChunk(Chunk* chunk, register O* target, O** targetOut) const {
			if (sizeof(I) == sizeof(O)) {
				memmove(target, chunk->data, chunk->length * sizeof(I));
				*targetOut = target + chunk->length;
			} else {
				register I* data = (I*) chunk->data;
				register Py_ssize_t l = chunk->length;
				while (l-- > 0) {
					target[l] = data[l];
				}
				*targetOut = target + chunk->length;
			}
		}

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


#endif

}

#endif /* FA06F563_6133_C709_128F_2D11FE8957D4 */
