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

	template<typename INTERNAL_TYPE, int64 MIN, int64 MAX>
	class RangedInteger
	{
		static_assert(MIN < MAX, "Min & Max values must be strictly ordered.");
		static_assert(MIN >= std::numeric_limits<INTERNAL_TYPE>::min(), "Min value out of bound");
		static_assert(MAX <= std::numeric_limits<INTERNAL_TYPE>::max(), "Max value out of bound");
	public:
		using Type = INTERNAL_TYPE;
		static constexpr Type Min() { return MIN; }
		static constexpr Type Max() { return MAX; }
		static constexpr uint8 NbBits = NbBits<MAX - MIN>::Value;

		RangedInteger() = default;
		RangedInteger(Type v) : mValue(v) { checkValue(); }
		template<typename OtherType>
		RangedInteger(OtherType v) : mValue(v) { checkValue(); }
		RangedInteger& operator=(Type v) { mValue = v; checkValue(); return *this; }
		template<typename OtherType>
		RangedInteger& operator=(OtherType v) { mValue = v; checkValue(); return *this; }

		static constexpr bool IsWithinRange(Type v) { return (v >= Min() && v <= Max()); }
		template<typename OtherType>
		static constexpr bool IsWithinRange(OtherType v) { return (v >= Min() && v <= Max()); }

		inline Type get() const { return mValue; }
		inline operator Type() const { return mValue; }

	private:
		void checkValue() { assert(IsWithinRange(mValue)); }

	private:
		Type mValue{ Min() };
	};


#define DEFINE_RANGED_TYPE(NAME, INTERNALTYPE)			\
	template<int64 MIN, int64 MAX>						\
	using NAME = RangedInteger<INTERNALTYPE, MIN, MAX>
	
	DEFINE_RANGED_TYPE(Int8, int8);
	DEFINE_RANGED_TYPE(Int16, int16);
	DEFINE_RANGED_TYPE(Int32, int32);

	DEFINE_RANGED_TYPE(UInt8, uint8);
	DEFINE_RANGED_TYPE(UInt16, uint16);
	DEFINE_RANGED_TYPE(UInt32, uint32);
#undef DEFINE_RANGED_TYPE

	template<uint8 BASE, uint8 EXPONENT>
	struct Pow
	{
		template<uint8 EXP>
		struct InternalPow
		{
			static constexpr uint32 Value = Return<uint32, BASE * InternalPow<EXP-1>::Value>::Value;
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

	template<class FLOATTYPE, int32 MIN, int32 MAX, uint8 NBDECIMALS, uint8 STEP = 1 >
	class Float
	{
		static_assert(NBDECIMALS > 0, "At least 1 decimal.");
		static_assert(NBDECIMALS < 10, "Maximum 10 decimals.");
		static_assert(STEP != 0, "Step must not be 0.");
		static_assert(STEP % 10 != 0, "Step should not be a multiple of 10. Remove a decimal.");
		using FloatType = FLOATTYPE;
		static constexpr int32 Min = MIN;
		static constexpr int32 Max = MAX;
		static constexpr uint32 Diff = Max - Min;
		static constexpr uint8 NbDecimals = NBDECIMALS;
		static constexpr uint32 Multiple = Pow<10, NbDecimals>::Value;
		static constexpr uint8 Step = STEP;
		static constexpr uint32 Domain = (MAX - MIN) * Multiple / STEP;

	public:
		Float() = default;
		Float(FloatType value)
		{
			mQuantizedValue = Quantize(value);
		}

		static uint32 Quantize(FloatType value)
		{
			assert(value >= Min && value <= Max);
			return static_cast<uint32>(((value - Min) * Multiple + 0.5) / Step);
		}

		inline FloatType get() const { return static_cast<FloatType>(mQuantizedValue.get() * Step * 1. / Multiple + Min); }
		inline operator FloatType() const { return get(); }

	private:
		UInt32<0, Domain> mQuantizedValue;
	};
}