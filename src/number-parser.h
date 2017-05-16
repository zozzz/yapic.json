#ifndef N5D751B0_4133_C707_176F_B6860048609B
#define N5D751B0_4133_C707_176F_B6860048609B

#include <limits>

#include "../libs/double-conversion/double-conversion/strtod.h"
#include "config.h"

namespace ZiboJson {


template<typename I>
struct NegativeNumberTrait {
	typedef I Int;

	static constexpr bool IsNegative = true;
	static constexpr Int Limit = std::numeric_limits<Int>::min() / 10;
	static constexpr Int FullLimit = std::numeric_limits<Int>::min();

	template<typename T>
	static inline const Int Convert(const T& ch) { return '0' - ch; };

	static inline const bool IsOk(const Int& value) {
		return Limit <= value;
	}
};


template<typename I>
struct PositiveNumberTrait {
	typedef I Int;

	static constexpr bool IsNegative = false;
	static constexpr Int Limit = std::numeric_limits<Int>::max() / 10;
	static constexpr Int FullLimit = std::numeric_limits<Int>::min();

	template<typename T>
	static inline const Int Convert(const T& ch) { return ch - '0'; };

	static inline const bool IsOk(const Int& value) {
		return Limit >= value;
	}
};


template<typename Input, typename Context, typename _Trait>
struct NumberFactory {
	typedef _Trait Trait;

	static inline typename Context::Result* NewInt(const typename Trait::Int& value) {
		return PyLong_FromLong(value);
	}

	static inline typename Context::Result* NewFloat(Context& ctx, Input* position);

	static inline typename Context::Result* NewInfinity() {
		if (Trait::IsNegative) {
			return PyFloat_FromDouble(-Py_HUGE_VAL);
		} else {
			return PyFloat_FromDouble(Py_HUGE_VAL);
		}
	}

	static inline typename Context::Result* NewNaN() {
		return PyFloat_FromDouble(Py_NAN);
	}

	static inline typename Context::Result* ThrowUnexpectedError(Input* position) {
		return NULL;
	}

	static inline typename Context::Result* ThrowOverflowError(Input* position) {
		return NULL;
	}

	static inline void OnNumberBegin(Context& ctx) {};

	static inline void AppendIntDigits(Context& ctx, Input* begin, Input* end) {
		while (begin < end && ctx.cursor < ctx.end) {
			*(ctx.cursor++) = *(begin++);
		}
	}

	// static inline void OnInt(Context& ctx, Input ch) {
	// 	if (Trait::IsOk(ctx.intValue)) {
	// 		ctx.intValue = ctx.intValue * 10 + Trait::Convert(ch);
	// 		*(ctx.cursor++) = ch;
	// 	} else {
	// 		ctx.intOverflow = true;
	// 		if (ctx.cursor < ctx.end) {
	// 			*(ctx.cursor++) = ch;
	// 		}
	// 	}
	// }

	static inline void OnDecimalPoint(Context& ctx, Input ch) {};

	static inline void OnFraction(Context& ctx, Input ch) {};

	static inline void OnNegativeExponentBegin(Context& ctx) {};

	static inline void OnPositiveExponentBegin(Context& ctx) {};

	static inline void OnExponent(Context& ctx, const Input ch, const int& digit);
};


template<typename Input, typename Context, typename Trait>
struct InternalNumberFactory: public NumberFactory<Input, Context, Trait> {
	static inline typename Context::Result* NewFloat(Context& ctx, Input* position) {
		if (ctx.cursor != ctx.end) {
			if (Trait::IsNegative) {
				return PyFloat_FromDouble(
					-double_conversion::Strtod(
						Vector<const char>(ctx.buffer, ctx.cursor - ctx.buffer),
						ctx.exponent + ctx.expTemp));
			} else {
				return PyFloat_FromDouble(
					double_conversion::Strtod(
						Vector<const char>(ctx.buffer, ctx.cursor - ctx.buffer),
						ctx.exponent + ctx.expTemp));
			}
		} else {
			return NumberFactory<Input, Context, Trait>::ThrowOverflowError(position);
		}
	}

	static inline void OnDecimalPoint(Context& ctx) { } // skip decimal point

	static inline void OnFraction(Context& ctx, Input ch) {
		if (ctx.cursor < ctx.end) {
			*(ctx.cursor++) = ch;
		}
		--ctx.exponent;
	}

	static inline void OnExponent(Context& ctx, const Input ch, const int& digit) {
		ctx.expTemp = ctx.expTemp * 10 + digit;
	}
};

template<typename Input, typename Context, typename Trait>
struct ExternalNumberFactory: public NumberFactory<Input, Context, Trait> {
	static inline typename Context::Result* NewFloat(Context& ctx, Input* position) {
		PyObject* str = PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, ctx.buffer, ctx.cursor - ctx.buffer);
		if (str != NULL) {
			PyObject* f = PyObject_CallFunctionObjArgs(ctx.parseFloat, str, NULL);
			Py_DECREF(str);
			return f;
		} else {
			return NULL;
		}
	}

	static inline void OnNumberBegin(Context& ctx) {
		if (Trait::IsNegative) {
			*(ctx.cursor++) = '-';
		}
	};

	static inline void OnDecimalPoint(Context& ctx) {
		if (ctx.cursor < ctx.end) {
			*(ctx.cursor++) = '.';
		}
	 }

	static inline void OnFraction(Context& ctx, Input ch) {
		if (ctx.cursor < ctx.end) {
			*(ctx.cursor++) = ch;
		}
	}

	static inline void OnNegativeExponentBegin(Context& ctx) {
		if (ctx.cursor + 2 < ctx.end) {
			*(ctx.cursor++) = 'e';
			*(ctx.cursor++) = '-';
		}
	};

	static inline void OnPositiveExponentBegin(Context& ctx) {
		if (ctx.cursor < ctx.end) {
			*(ctx.cursor++) = 'e';
		}
	};

	static inline void OnExponent(Context& ctx, const Input ch, const int& digit) {
		if (ctx.cursor < ctx.end) {
			*(ctx.cursor++) = ch;
		}
	}
};


template<typename Input, typename Context, typename PositiveFactory, typename NegativeFactory>
struct NumberRader {
	// typedef PositiveFactory PositiveFactory;
	// typedef NegativeFactory NegativeFactory;
	typedef NegativeNumberTrait<int> NegativeExponent;
	typedef PositiveNumberTrait<int> PositiveExponent;

	public:
		static inline typename Context::Result* Read(Input* in, Input** out, Context& ctx) {
			if (IsNegative(in, &in)) {
				NegativeFactory::OnNumberBegin(ctx);
				return ReadNumber<NegativeFactory>(in, out, ctx);
			} else {
				PositiveFactory::OnNumberBegin(ctx);
				return ReadNumber<PositiveFactory>(in, out, ctx);
			}
		}

	private:
		static inline bool IsNegative(Input* in, Input** out) {
			if (*in == '-') {
				*out = ++in;
				return true;
			} else if (*in == '+') {
				*out = ++in;
			}
			return false;
		}

		template<typename Factory>
		static inline typename Context::Result* ReadNumber(Input* in, Input** out, Context& ctx) {
			Factory::Trait::Int intValue = 0;
			bool intOverflow = false;

			if (IsZero<Factory>(in, out, ctx) || ReadInt<Factory>(in, out, &intValue, &intOverflow)) {
				Factory::AppendIntDigits(ctx, in, *out);
				if (ReadFloat<Factory>(*out, out, ctx) || intOverflow) {
					return Factory::NewFloat(ctx, in);
				} else {
					return Factory::NewInt(intValue);
				}
			} else if (ReadInfinity(in, out)) {
				return Factory::NewInfinity();
			} else if (ReadNaN(in, out)) {
				return Factory::NewNaN();
			} else {
				return Factory::ThrowUnexpectedError(in);
			}
		}

		template<typename Factory>
		static inline bool IsZero(Input* in, Input** out, Context& ctx) {
			if (*in == '0') {
				*out = ++in;
				return true;
			} else {
				return false;
			}
		}

		template<typename Factory>
		static inline bool ReadInt(Input* in, Input** out, typename Factory::Trait::Int* intValue, bool* overflow) {
			if (*in >= '0' && *in <= '9') {
				do {
					*intValue = *intValue * 10 + Factory::Trait::Convert(*(in++));
				} while (*in >= '0' && *in <= '9' && Factory::Trait::IsOk(*intValue));

				if (*in >= '0' && *in <= '9') {
					*overflow = true;
					do {
						++in;
					} while (*in >= '0' && *in <= '9');
				}
				*out = in;
				return true;
			} else {
				return false;
			}
		}

		template<typename Factory>
		static inline bool ReadFloat(Input* in, Input** out, Context& ctx) {
			ReadFraction<Factory>(in, &in, ctx);
			ReadExponent<Factory>(in, &in, ctx);
			if (in != *out) {
				*out = in;
				return true;
			} else {
				return false;
			}
		}

		template<typename Factory>
		static inline bool ReadFraction(Input* in, Input** out, Context& ctx) {
			if (*(in++) == '.') {
				Factory::OnDecimalPoint(ctx);
				if (*in >= '0' && *in <= '9') {
					do {
						Factory::OnFraction(ctx, *(in++));
					} while (*in >= '0' && *in <= '9');
					*out = in;
					return true;
				}
			}
			return false;
		}

		template<typename Factory>
		static inline bool ReadExponent(Input* in, Input** out, Context& ctx) {
			if (*in == 'e' || *in == 'E') {
				if (IsNegative(++in, &in)) {
					Factory::OnNegativeExponentBegin(ctx);
					return __read_exponent<Factory, NegativeExponent>(in, out, ctx);
				} else {
					Factory::OnPositiveExponentBegin(ctx);
					return __read_exponent<Factory, PositiveExponent>(in, out, ctx);
				}
			} else {
				return false;
			}
		}

		template<typename Factory, typename ETrait>
		static inline bool __read_exponent(Input* in, Input** out, Context& ctx) {
			if (*in >= '0' && *in <= '9') {
				do {
					Factory::OnExponent(ctx, *in, ETrait::Convert(*in));
				} while (*(++in) >= '0' && *in <= '9');
				*out = in;
				return true;
			} else {
				return false;
			}
		}

		static inline bool ReadInfinity(Input* in, Input** out) {
			if (in[0] == 'I' && in[1] == 'n' && in[2] == 'f' && in[3] == 'i' &&
				in[4] == 'n' && in[5] == 'i' && in[6] == 't' && in[7] == 'y') {
				*out = in + 8;
				return true;
			} else {
				return false;
			}
		}

		static inline bool ReadNaN(Input* in, Input** out) {
			if (in[0] == 'N' && in[1] == 'a' && in[2] == 'N') {
				*out = in + 3;
				return true;
			} else {
				return false;
			}
		}

};

template<typename Input, typename Context>
struct InternalNumberReader: public NumberRader<
		Input,
		Context,
		InternalNumberFactory<Input, Context, PositiveNumberTrait<long long>>,
		InternalNumberFactory<Input, Context, NegativeNumberTrait<long long>>
	> { };

template<typename Input, typename Context>
struct ExternalNumberReader: public NumberRader<
		Input,
		Context,
		ExternalNumberFactory<Input, Context, PositiveNumberTrait<long long>>,
		ExternalNumberFactory<Input, Context, NegativeNumberTrait<long long>>
	> { };






















struct ___NumberReader {
	template<bool V>
	struct IsNegative {
		static constexpr bool Minus = V;
		operator bool() const { return V; }
	};


	/*
	template<typename T>
	Factory
		typedef Result;

		Result NewInt(long long value)

		template<bool isNegative>
		Result NewFloat(T* begin, T* end, T* decimalPoint, int exponent);

		Result NewFloatZero();

		template<bool isNegative>
		Result NewInfinity();

		template<bool isNegative>
		Result NewNaN();
	*/


	template<typename T, typename Factory>
	static inline bool Read(T* in, T** out, typename Factory::Result** result) {
		if (ReadSign(in, &in)) {
			return __read_number<T, NegativeNumber<T, long long>, Factory>(in, out, result);
		} else {
			return __read_number<T, PositiveNumber<T, long long>, Factory>(in, out, result);
		}
	}

	private:
		template<typename T, T Val>
		struct Const {
			static constexpr T Value = Val;
		};

		template<typename T, typename V>
		struct PositiveNumber {
			typedef V Type;
			typedef Const<V, std::numeric_limits<V>::max() / 10> Limit;
			typedef IsNegative<false> Sign;

			static inline V Convert(const T& value) {
				return value - '0';
			}

			static inline bool IsUnderMax(const V& value) {
				return Limit::Value <= value;
			}
		};

		template<typename T, typename V>
		struct NegativeNumber {
			typedef V Type;
			typedef Const<V, std::numeric_limits<V>::min() / 10> Limit;
			typedef IsNegative<true> Sign;

			static inline V Convert(const T& value) {
				return '0' - value;
			}

			static inline bool IsUnderMax(const V& value) {
				return Limit::Value <= value;
			}
		};

		template<typename T>
		static inline bool ReadSign(T* in, T** out) {
			if (*in == '-') {
				*out = ++in;
				return true;
			} else if (*in == '+') {
				*out = ++in;
			}
			return false;
		}

		template<typename T, class Trait, typename Factory>
		static inline bool __read_number(T* in, T** out, typename Factory::Result** result) {
			long long longValue = 0;
			bool overflow = false;

			if (__read_int<T, Trait>(in, out, &longValue, &overflow)) {
				T* floatBegin = Trait::Sign::Minus ? in - 1 : in;
				if (__read_float<T, Trait, Factory>(*out, out, floatBegin, result)) {
					return true;
				} else if (overflow == false) {
					*result = Factory::NewInt(longValue);
					return true;
				} else {
					return false;
				}
			} else if (__read_infinity(in, out)) {
				*result = Factory::NewInfinity();
				return true;
			} else if (__read_nan(in, out)) {
				*result = Factory::NewNaN();
				return true;
			} else {
				return false;
			}
		}

		template<typename T, class Trait>
		static inline bool __read_int(T* in, T** out, typename Trait::Type* value, bool* overflow) {
			if (*in >= '0' && *in <= '9') {
				*value = (*value * 10) + Trait::Convert(*(in++));
				while (*in >= '0' && *in <= '9' && *value <= Trait::Limit::Value) {
					*value = (*value * 10) + Trait::Convert(*(in++));
				}
				if (*in >= '0' && *in <= '9') {
					*overflow = true;
					do {
						in++;
					} while (*in >= '0' && *in <= '9');
				}
				*out = in;
				return true;
			} else {
				return false;
			}
		}

		template<typename T, class Trait, typename Factory>
		static inline bool __read_float(T* in, T** out, T* begin, typename Factory::Result** result) {
			int exponent = 0;
			T* decimalPoint = in;

			if (__read_fraction(in, &in, &exponent) == false) {
				decimalPoint = NULL;
			}

			int e = 0;
			if (__read_exponent(in, &in, &e)) {
				exponent += e;
			}

			if (in != *out) {
				*result = Factory::NewFloat(begin, in, decimalPoint, exponent);
				*out = in;
				return true;
			} else {
				return false;
			}
		}

		template<typename T>
		static inline bool __read_fraction(T* in, T** out, int* exponent) {
			if (*(in++) == '.') {
				if (*in == '0') {
					do {
						in++;
						(*exponent)--;
					} while (*in == '0');
				}

				while (*in >= '0' && *in <= '9') {
					in++;
					(*exponent)--;
				}
				// *exponent -= in - *out;
				*out = in;
				return true;
			} else {
				return false;
			}
		}

		template<typename T>
		static inline bool __read_exponent(T* in, T** out, int* exponent) {
			if (*in == 'e' || *in == 'E') {
				bool overflow = false;
				if (ReadSign(in, &in)
					? __read_int<T, NegativeNumber<T, int>>(in, out, exponent, &overflow)
					: __read_int<T, PositiveNumber<T, int>>(in, out, exponent, &overflow)) {
					return !overflow;
				} else {
					return false;
				}
			} else {
				return false;
			}
		}

		template<typename T>
		static inline bool __read_infinity(T* in, T** out) {
			if (in[0] == 'I' && in[1] == 'n' && in[2] == 'f' && in[3] == 'i' &&
				in[4] == 'n' && in[5] == 'i' && in[6] == 't' && in[7] == 'y') {
				*out = in + 8;
				return true;
			} else {
				return false;
			}
		}

		template<typename T>
		static inline bool __read_nan(T* in, T** out) {
			if (in[0] == 'N' && in[1] == 'a' && in[2] == 'N') {
				*out = in + 3;
				return true;
			} else {
				return false;
			}
		}
};

}

#endif /* N5D751B0_4133_C707_176F_B6860048609B */
