#pragma once

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
			static constexpr uint8 Value = std::conditional<VALUE < (Bit<uint64>::Right << BIT), Return<uint8, BIT>, InternalNbBits<BIT + 1>>::type::Value;
		};
		static constexpr uint8 Value = std::conditional<VALUE >= (std::numeric_limits<uint64>::max() / 2 + 1), Return<uint8, 64>, InternalNbBits<1>>::type::Value;
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
}