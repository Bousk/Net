#pragma once

#define NOMINMAX
#include <cstdint>
#include <cassert>
#include <limits>

namespace Bousk
{
	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;

	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

	using float32 = float;
	using float64 = double;

	template<class INTEGER>
	struct Bit {};
	template<>
	struct Bit<uint8>
	{
		static constexpr uint8 Left = 0b10000000;
		static constexpr uint8 Right = 0b00000001;
	};
	constexpr uint8 BoolTrue = Bit<uint8>::Right;
	constexpr uint8 BoolFalse = 0;

	template<>
	struct Bit<uint64>
	{
		static constexpr uint64 Right = 0b0000000000000000000000000000000000000000000000000000000000000001;
	};

	template<typename TYPE, TYPE V>
	struct Return
	{
		static constexpr TYPE Value = V;
	};

	template<uint64 VALUE>
	struct NbBits
	{
		template<uint8 BIT>
		struct InternalNbBits
		{
			static constexpr uint8 Value = std::conditional_t<VALUE < (Bit<uint64>::Right << BIT), Return<uint8, BIT>, InternalNbBits<BIT + 1>>::Value;
		};
		static constexpr uint8 Value = std::conditional_t<VALUE >= (std::numeric_limits<uint64>::max() / 2 + 1), Return<uint8, 64>, InternalNbBits<1>>::Value;
	};

	template<uint8 BASE, uint8 EXPONENT>
	struct Pow
	{
		template<uint8 EXP>
		struct InternalPow
		{
			static constexpr uint32 Value = Return<uint32, BASE * InternalPow<EXP - 1>::Value>::Value;
		};
		template<>
		struct InternalPow<1>
		{
			static constexpr uint32 Value = BASE;
		};
		template<>
		struct InternalPow<0>
		{
			static constexpr uint32 Value = 1;
		};
		static constexpr uint32 Value = InternalPow<EXPONENT>::Value;
	};

	template<char...> struct ToInt;
	template<>
	struct ToInt<>
	{
		static constexpr int Value = 0;
	};
	template<char C>
	struct ToInt<C>
	{
		static_assert(C == '0' || C == '1' || C == '2' || C == '3' || C == '4' || C == '5' || C == '6' || C == '7' || C == '8' || C == '9', "Unexpected value");
		static constexpr int Value = C - '0';
	};
	template<char C, char... VALUES>
	struct ToInt<C, VALUES...>
	{
		static constexpr int Value = ToInt<C>::Value * Pow<10, sizeof...(VALUES)>::Value + ToInt<VALUES...>::Value;
	};

#define DEFINE_INTEGER_LITERAL(SUFFIX, INTERNALTYPE)	\
	template<char... VALUES>							\
	constexpr INTERNALTYPE operator "" SUFFIX() { static_assert(ToInt<VALUES...>::Value >= std::numeric_limits<INTERNALTYPE>::min() && ToInt<VALUES...>::Value <= std::numeric_limits<INTERNALTYPE>::max(), ""); return static_cast<INTERNALTYPE>(ToInt<VALUES...>::Value); }

	namespace Literals
	{
		DEFINE_INTEGER_LITERAL(_i8, int8);
		DEFINE_INTEGER_LITERAL(_i16, int16);
		DEFINE_INTEGER_LITERAL(_i32, int32);

		DEFINE_INTEGER_LITERAL(_u8, uint8);
		DEFINE_INTEGER_LITERAL(_u16, uint16);
		DEFINE_INTEGER_LITERAL(_u32, uint32);
	}

#undef DEFINE_INTEGER_LITERAL
}