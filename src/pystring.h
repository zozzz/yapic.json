#ifndef E00E3A82_9133_C707_1616_95D1A54DDD8A
#define E00E3A82_9133_C707_1616_95D1A54DDD8A

#include <algorithm>


#include "config.h"
#include "util.h"

namespace ZiboJson {

template<typename T>
struct PyUnicode_Traits;


template<>
struct PyUnicode_Traits<Py_UCS1> {
	static const PyUnicode_Kind Kind = PyUnicode_1BYTE_KIND;
	static const Py_UCS1 From = 0;
	static const Py_UCS1 BasicMax = 0x7F;
	static const Py_UCS1 Max = 0xFF;

	template<typename MCH>
	static const inline void UpdateMaxChar(MCH* maxchar, Py_UCS1 const& ch) {
		// if (ch > BasicMax) {
		// 	*maxchar = Max;
		// }
	}


#if ZIBO_JSON_USE_SSE
	// static const __m128i& MaxCharMask() {
	// 	static const __m128i mask = _mm_set_epi16(0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF);
	// 	return mask;
	// }

	static inline const Py_UCS1 DetermineMaxChar(Py_UCS1* begin, Py_ssize_t l) {
		while (l >= 16) {
			__m128i chunk = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(begin));
			if (_mm_movemask_epi8(chunk)) {
				return Max;
			} else {
				begin += 16;
				l -= 16;
			}
		}


		while (l-- > 0) {
			if (*(begin++) > 0x7F) {
				return Max;
			}
		}

		return 0x7F;
	}

#else

	static const Py_UCS1 ExtAsciiMask = 0x80;

	static const Py_UCS1 DetermineMaxChar(register Py_UCS1* begin, Py_ssize_t l) {
		register Py_UCS1 *end = begin + ZiboJson_UnrolledLoopEnd(l, 8);
		while (begin < end) {
			if ((begin[0] | begin[1] | begin[2] | begin[3] |
				 begin[4] | begin[5] | begin[6] | begin[7]) & ExtAsciiMask) {
				return Max;
			} else {
				begin += 8;
			}
		}

		end = begin + l;
		while (begin < end) {
			if (*(begin++) & ExtAsciiMask) {
				return Max;
			}
		}
		return BasicMax;
	}

#endif
};


template<>
struct PyUnicode_Traits<Py_UCS2> {
	static const PyUnicode_Kind Kind = PyUnicode_2BYTE_KIND;
	static const Py_UCS2 From = PyUnicode_Traits<Py_UCS1>::Max;
	static const Py_UCS2 Max = 0xFFFF;

	template<typename MCH>
	static const inline void UpdateMaxChar(MCH* maxchar, Py_UCS2 const& ch) {
		if (*maxchar != Max) {
			*maxchar = ch & 0x700
				? Max
				: PyUnicode_Traits<Py_UCS1>::Max;
		}
	}


#if ZIBO_JSON_USE_SSE
	static inline const __m128i& MaxCharMask() {
		static const __m128i mask = _mm_set_epi16(0x700, 0x700, 0x700, 0x700, 0x700, 0x700, 0x700, 0x700);
		return mask;
	}

	static inline const Py_UCS2 DetermineMaxChar(Py_UCS2* begin, Py_ssize_t l) {
		const __m128i mask = MaxCharMask();
		while (l >= 8) {
			l -= 8;
			__m128i chunk = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(begin + l));
			if (_mm_test_all_zeros(chunk, mask) == 0) {
				return Max;
			}
		}

		while (l-- > 0) {
			if (begin[l] > From) {
				return Max;
			}
		}

		return PyUnicode_Traits<Py_UCS1>::Max;
	}
#else

	static inline const Py_UCS2 DetermineMaxChar(Py_UCS2* begin, Py_ssize_t l) {
		return *std::max_element(begin, begin + l);
	}

#endif
};


template<>
struct PyUnicode_Traits<Py_UCS4> {
	static const PyUnicode_Kind Kind = PyUnicode_4BYTE_KIND;
	static const Py_UCS4 From = PyUnicode_Traits<Py_UCS2>::Max;
	static const Py_UCS4 Max = 0x10FFFF;

	template<typename MCH>
	static const inline void UpdateMaxChar(MCH* maxchar, Py_UCS4 const& ch) {
		if (*maxchar != Max) {
			if (ch & 0xFFF800) {
				*maxchar = Max;
			} else if (*maxchar < ch) {
				*maxchar = ch;
			}
		}
	}


#if ZIBO_JSON_USE_SSE
	static inline const __m128i& MaxCharMask() {
		static const __m128i mask = _mm_set_epi32(0xFFF800, 0xFFF800, 0xFFF800, 0xFFF800);
		return mask;
	}

	static inline const Py_UCS4 DetermineMaxChar(Py_UCS4* begin, Py_ssize_t l) {
		Py_UCS4 maxchar = PyUnicode_Traits<Py_UCS1>::Max;

		while (l >= 4) {
			__m128i chunk = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(begin));
			if (_mm_test_all_zeros(chunk, MaxCharMask()) == 0) {
				return Max;
			} else if (_mm_test_all_zeros(chunk, PyUnicode_Traits<Py_UCS2>::MaxCharMask()) == 0) {
				maxchar = PyUnicode_Traits<Py_UCS2>::Max;
			}
			begin += 4;
			l -= 4;
		}

		while (l-- > 0) {
			if (*begin > From) {
				return Max;
			} else if (*begin > PyUnicode_Traits<Py_UCS2>::From) {
				maxchar = PyUnicode_Traits<Py_UCS2>::Max;
			}
			++begin;
		}

		return maxchar;
	}

#else

	static inline const Py_UCS4 DetermineMaxChar(Py_UCS4* begin, Py_ssize_t l) {
		return *std::max_element(begin, begin + l);
	}

#endif
};

}

#endif /* E00E3A82_9133_C707_1616_95D1A54DDD8A */
