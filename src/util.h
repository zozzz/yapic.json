#ifndef U492A975_4133_C6A2_1678_FF0AE59B4CBD
#define U492A975_4133_C6A2_1678_FF0AE59B4CBD

#include "config.h"

#define ZiboJson_UnrolledLoopEnd(length, loopSize) ((length) & ~((loopSize) - 1))

namespace ZiboJson {


#define __bytelength(T, l) \
	(sizeof(T) == 1 ? l : sizeof(T) == 2 ? l << 1 : l << 2)


template<typename I, typename O, typename S>
static inline void CopyBytes(O* dest, const I* input, S length) {
	assert(dest != NULL);
	assert(input != NULL);
	assert(sizeof(I) == 1 || sizeof(I) == 2 || sizeof(I) == 4);

	if (sizeof(I) == sizeof(O)) {
		memcpy(dest, input, __bytelength(I, length));
	} else {
		while (length-- > 0) {
			dest[length] = input[length];
		}
	}
}


template<typename I, typename O, typename S>
static inline void MoveBytes(O* dest, const I* input, S length) {
	assert(dest != NULL);
	assert(input != NULL);
	assert(sizeof(I) == 1 || sizeof(I) == 2 || sizeof(I) == 4);

	if (sizeof(I) == sizeof(O)) {
		memmove(dest, input, __bytelength(I, length));
	} else {
		while (length-- > 0) {
			dest[length] = input[length];
		}
	}
}

#undef __bytelength

} /* end namespace ZiboJson */

#endif /* U492A975_4133_C6A2_1678_FF0AE59B4CBD */
