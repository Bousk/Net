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

	template<typename INTERNAL_TYPE, INTERNAL_TYPE MIN = std::numeric_limits<INTERNAL_TYPE>::min(), INTERNAL_TYPE MAX = std::numeric_limits<INTERNAL_TYPE>::max()>
	class RangedInteger
	{
		static_assert(MIN < MAX, "Min & Max values must be strictly ordered");
	public:
		using Type = INTERNAL_TYPE;
		static constexpr Type Min = MIN;
		static constexpr Type Max = MAX;

		RangedInteger() = default;
		RangedInteger(Type v) : mValue(v) { checkValue(); }
		template<typename OtherType>
		RangedInteger(OtherType v) : mValue(v) { checkValue(); }
		RangedInteger& operator=(Type v) { mValue = v; checkValue(); return *this; }
		template<typename OtherType>
		RangedInteger& operator=(OtherType v) { mValue = v; checkValue(); return *this; }

		static constexpr bool IsWithinRange(Type v) { return (v >= Min && v <= Max); }
		template<typename OtherType>
		static constexpr bool IsWithinRange(OtherType v) { return (v >= Min && v <= Max); }

	private:
		void checkValue() { assert(IsWithinRange(mValue)); }

	private:
		Type mValue{ Min };
	};


#define DEFINE_RANGED_TYPE(NAME, INTERNALTYPE) \
	template<INTERNALTYPE MIN = std::numeric_limits<INTERNALTYPE>::min(), INTERNALTYPE MAX = std::numeric_limits<INTERNALTYPE>::max()> \
	using NAME = RangedInteger<INTERNALTYPE, MIN, MAX>

	DEFINE_RANGED_TYPE(Int8, int8);
	DEFINE_RANGED_TYPE(Int16, int16);
	DEFINE_RANGED_TYPE(Int32, int32);

	DEFINE_RANGED_TYPE(Uint8, uint8);
	DEFINE_RANGED_TYPE(Uint16, uint16);
	DEFINE_RANGED_TYPE(Uint32, uint32);

#undef DEFINE_RANGED_TYPE
}