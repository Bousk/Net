#pragma once

#include <Types.hpp>
#include <Serialization/Serialization.hpp>
#include <Serialization/Serializer.hpp>

namespace Bousk
{
	template<typename INTERNAL_TYPE, int64 MIN, int64 MAX>
	class RangedInteger : public Serialization::Serializable
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

		bool write(Serialization::Serializer& serializer) const override { return serializer.write(get(), Min(), Max()); }
		bool read(Serialization::Deserializer& deserializer) override { return deserializer.read(mValue, Min(), Max()); }

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
}