#ifndef C1A64348_5133_C6A2_15F7_4A8FAE5B356E
#define C1A64348_5133_C6A2_15F7_4A8FAE5B356E

#ifndef NDEBUG
#	include <iostream>
#	include <bitset>
#endif

#include "emmintrin.h"
#include "nmmintrin.h"
#include "config.h"

namespace ZiboJson { namespace sse {

union M128 {
	char i8[16];
	__m128i i128;
};

#ifndef NDEBUG
	template<typename V>
	void DUMP_M128(V v) {
		assert(0);
	}

	template<>
	void DUMP_M128<__m128i>(__m128i value) {
		// TODO: átalakítani char poiunterre, majd azt dumpolni
	}

	template<>
	void DUMP_M128<M128>(M128 v) {
		DUMP_M128(v.i128);
	}
#else
#	define DUMP_M128(v) ((void)0)
#endif





template<typename O, typename I>
static inline void string_copy(O* out, I* input, size_t size) {
	if (sizeof(O) == sizeof(I)) {
		memcpy(out, input, size);
	} else {
		size_t consumed = 0;
		if (size >= 16) {
			size_t writed = 0;
			__m128i shuffle;
			M128 shuffled;

			switch (sizeof(I)) {
				case 1:
					switch (sizeof(O)) {
						case 2:
							shuffle = _mm_set_epi8(0xFF, 7, 0xFF, 6, 0xFF, 5, 0xFF, 4, 0xFF, 3, 0xFF, 2, 0xFF, 1, 0xFF, 0);
						break;

						case 4:
							shuffle = _mm_set_epi8(0xFF, 0xFF, 0xFF, 3, 0xFF, 0xFF, 0xFF, 2, 0xFF, 0xFF, 0xFF, 1, 0xFF, 0xFF, 0xFF, 0);
						break;

						default:
							assert(0);
						break;
					}
				break;

				case 2:
					switch (sizeof(O)) {
						case 1:
							shuffle = _mm_set_epi8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 14, 12, 10, 8, 6, 4, 2, 0);
						break;

						case 4:
							shuffle = _mm_set_epi8(0xFF, 0xFF, 0, 1, 0xFF, 0xFF, 2, 3, 0xFF, 0xFF, 4, 5, 0xFF, 0xFF, 6, 7);
						break;

						default:
							assert(0);
						break;
					}
				break;

				case 4:
					switch (sizeof(O)) {
						case 1:
							shuffle = _mm_set_epi8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 12, 8, 4, 0);
						break;

						case 2:
							shuffle = _mm_set_epi8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 13, 12, 9, 8, 5, 4, 1, 0);
						break;

						default:
							assert(0);
						break;
					}
				break;

				default:
					assert(0);
				break;
			}

			do {
				shuffled.i128 = _mm_shuffle_epi8(
					_mm_lddqu_si128(reinterpret_cast<const __m128i*>(reinterpret_cast<const char*>(input) + consumed)),
					shuffle);

				DUMP_M128(shuffled);

				switch (sizeof(I)) {
					case 1:
						switch (sizeof(O)) {
							case 2:
								memcpy(reinterpret_cast<char*>(out) + writed, shuffled.i8, 16);
								writed += 16;
								consumed += 8;
							break;

							case 4:
								memcpy(reinterpret_cast<char*>(out) + writed, shuffled.i8, 16);
								writed += 16;
								consumed += 4;
							break;
						}
					break;

					case 2:
						switch (sizeof(O)) {
							case 1:
								memcpy(reinterpret_cast<char*>(out) + writed, shuffled.i8, 8);
								writed += 8;
								consumed += 16;
							break;

							case 4:
								memcpy(reinterpret_cast<char*>(out) + writed, shuffled.i8, 4);
								writed += 4;
								consumed += 8;
							break;
						}
					break;

					case 4:
						switch (sizeof(O)) {
							case 1:
								memcpy(reinterpret_cast<char*>(out) + writed, shuffled.i8, 4);
								writed += 4;
								consumed += 16;
							break;

							case 2:
								memcpy(reinterpret_cast<char*>(out) + writed, shuffled.i8, 8);
								writed += 8;
								consumed += 16;
							break;
						}
					break;

				}

			} while (consumed + 16 < size);
		}

		// copy remaining data
		size /= sizeof(I);
		consumed /= sizeof(I);
		for (; consumed < size; consumed++) {
			out[consumed] = input[consumed];
		}
	}
}

} /* end namespace sse */
} /* end namespace ZiboJson */

#endif /* C1A64348_5133_C6A2_15F7_4A8FAE5B356E */
