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
		static constexpr bool IsInternalTypeValid = std::is_same<INTERNAL_TYPE, int8>::value
			|| std::is_same<INTERNAL_TYPE, int16>::value
			|| std::is_same<INTERNAL_TYPE, int32>::value
			|| std::is_same<INTERNAL_TYPE, uint8>::value
			|| std::is_same<INTERNAL_TYPE, uint16>::value
			|| std::is_same<INTERNAL_TYPE, uint32>::value;
		static_assert(IsInternalTypeValid, "RangedInteger can only be used with int8, int16, int32, uint8, uint16 or uint32");
	public:
		using Type = INTERNAL_TYPE;
		static constexpr Type Min() { return MIN; }
		static constexpr Type Max() { return MAX; }
		static constexpr uint8 NbBits = NbBits<MAX - MIN>::Value;

		RangedInteger() = default;
		explicit RangedInteger(Type v) : mValue(v) { checkValue(); }
		RangedInteger& operator=(Type v) { CheckValue(v); mValue = v; return *this; }
		template<typename OtherType>
		RangedInteger(OtherType v) { CheckValue(v); mValue = static_cast<Type>(v); }
		template<typename OtherType>
		RangedInteger& operator=(OtherType v) { CheckValue(v); mValue = static_cast<Type>(v); return *this; }

		static constexpr bool IsWithinRange(Type v) { return (v >= Min() && v <= Max()); }
		template<typename OtherType>
		static constexpr bool IsWithinRange(OtherType v)
		{
			using CastType = int64; // Cast to int64 which can accept any types we use RangedInteger for to prevent any warnings
			return (static_cast<CastType>(v) >= static_cast<CastType>(Min()) && static_cast<CastType>(v) <= static_cast<CastType>(Max()));
		}

		inline Type get() const { return mValue; }
		inline operator Type() const { return mValue; }

		bool write(Serialization::Serializer& serializer) const override { return serializer.write(get(), Min(), Max()); }
		bool read(Serialization::Deserializer& deserializer) override { return deserializer.read(mValue, Min(), Max()); }

	private:
		void checkValue() { assert(IsWithinRange(mValue)); }
		static void CheckValue(Type v) { assert(IsWithinRange(v)); }
		template<typename OtherType>
		void CheckValue(OtherType v) { assert(IsWithinRange(v)); }

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