#pragma once

#include <Types.hpp>
#include <Serialization/Serialization.hpp>
#include <Serialization/Serializer.hpp>
#include <Serialization/Deserializer.hpp>
#include <Utils.hpp>

namespace Bousk
{
	// Extract a type from a value
	// Extract the most fitting type, ie : -1 => int8, 257 => uint16, ...
	template<auto V, bool ForceUnsigned>
	struct ExtractType
	{
		static_assert(std::numeric_limits<decltype(V)>::is_integer, "ExtractType must be used with integer type");
		static_assert(!std::is_same_v<decltype(V), bool>, "ExtractType must not be used with bool");
		using Type = std::conditional_t< V < 0
			, std::conditional_t< V < std::numeric_limits<int32>::min(), int64
			, std::conditional_t< V < std::numeric_limits<int16>::min(), int32
			, std::conditional_t< V < std::numeric_limits<int8>::min(), int16
			, int8
			>>>
			// > 0 : force unsigned type ?
			, std::conditional_t< (V > std::numeric_limits<int64>::max()), uint64
			, std::conditional_t< (V > std::numeric_limits<uint32>::max()), std::conditional_t<ForceUnsigned, uint64, int64>
			, std::conditional_t< (V > std::numeric_limits<int32>::max()), std::conditional_t<ForceUnsigned, uint32, int64>
			, std::conditional_t< (V > std::numeric_limits<uint16>::max()), std::conditional_t<ForceUnsigned, uint32, int32>
			, std::conditional_t< (V > std::numeric_limits<int16>::max()), std::conditional_t<ForceUnsigned, uint16, int32>
			, std::conditional_t< (V > std::numeric_limits<uint8>::max()), std::conditional_t<ForceUnsigned, uint16, int16>
			, std::conditional_t< (V > std::numeric_limits<int8>::max()), std::conditional_t<ForceUnsigned, uint8, int16>
			, std::conditional_t<ForceUnsigned, uint8, int8>
			>>>>>>>
			>;
	};
	// Promote a type to a bigger one (int32 => int64, uint8 => uint16, ...)
	template<class T>
	struct Promote { using Type = std::conditional_t<std::is_signed_v<T>, int64, uint64>; }; // Default, use biggest type available. Should not be used be needed to compile
	template<> struct Promote<int32> { using Type = int64; };
	template<> struct Promote<uint32> { using Type = uint64; };
	template<> struct Promote<int16> { using Type = int32; };
	template<> struct Promote<uint16> { using Type = uint32; };
	template<> struct Promote<int8> { using Type = int16; };
	template<> struct Promote<uint8> { using Type = uint16; };
	// Return the biggest type
	template<class A, class B>
	struct Biggest
	{
		using Type = std::conditional_t<(sizeof(A) > sizeof(B)), A, B>;
	};
	// Find type fitting both A & B
	template<class A, class B>
	struct HoldingType
	{
		// Is it possible at all to find a fitting type for those 2 values ?
		// The only way it wouldn't is if one is uint64 while the other is signed
		static constexpr bool IsPossible = !((std::is_same_v<A, uint64> && std::is_signed_v<B>) || (std::is_same_v<B, uint64> && std::is_signed_v<A>));

		using Type = typename std::conditional_t<!IsPossible, void
			// They are same type : just use that type
			, std::conditional_t<std::is_same_v<A, B>, A
			// Same signed, use the biggest one
			, std::conditional_t<std::is_signed_v<A> == std::is_signed_v<B>, typename Biggest<A, B>::Type
			// Biggest is signed, use that one
			, std::conditional_t<std::is_signed_v<typename Biggest<A, B>::Type>, typename Biggest<A, B>::Type
			// Otherwise, use signed bigger than biggest
			, std::make_signed_t<typename Promote<typename Biggest<A, B>::Type>::Type>
			>>>>;
	};
	// Extract a type capable of holding both values, if possible
	template<auto MIN, auto MAX>
	struct FittingType
	{
		static_assert(MIN < MAX);

		using MinType = typename ExtractType<MIN, (MIN >= 0)>::Type;
		using MaxType = typename ExtractType<MAX, (MIN >= 0)>::Type;
		// < MAX > int64 max would need a uint64, MIN < 0 would need a signe type : impossible
		static constexpr bool IsPossible = !(MIN < 0 && MAX > std::numeric_limits<int64>::max()) || HoldingType<MinType, MaxType>::IsPossible;
					   
		using Type =
			std::conditional_t<!IsPossible, void,
			// Can MAX be hold by MinType ?
			std::conditional_t<(MAX <= std::numeric_limits<MinType>::max()), MinType,
			// Can MIN be hold by MaxType ?
			std::conditional_t<(MIN >= std::numeric_limits<MaxType>::min()), MaxType,
			// If not, find a type big enough to accomodate both values
			typename HoldingType<MinType, MaxType>::Type
			>>>;
	};

	template<auto MIN, auto MAX>
	constexpr uint64 Range()
	{
		static_assert(MIN < MAX);
		if constexpr (MAX < 0)
		{
			// Both < 0
			return static_cast<uint64>(static_cast<int64>(-1)* MIN) - static_cast<uint64>(static_cast<int64>(-1)* MAX);
		}
		else if constexpr (MIN < 0)
		{
			#pragma warning(push)
			#pragma warning(disable: 4307) // '*': signed integral constant overflow
			return static_cast<uint64>(MAX) + static_cast<uint64>(static_cast<int64>(-1)* MIN);
			#pragma warning(pop)
		}
		else
		{
			return static_cast<uint64>(MAX) - static_cast<uint64>(MIN);
		}
	}

	template<auto MIN, auto MAX>
	class RangedInteger : public Serialization::Serializable
	{
		static_assert(MIN < MAX, "Min & Max values must be strictly ordered");
		static_assert(FittingType<MIN, MAX>::IsPossible, "Min & Max value impossible to fit in any type");
	public:
		using Type = typename FittingType<MIN, MAX>::Type;
		static constexpr Type Min() { return MIN; }
		static constexpr Type Max() { return MAX; }
		static constexpr uint64 Range = Range<MIN, MAX>();
		static constexpr uint8 NbBits = NbBits<Range>::Value;

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
			if constexpr (!HoldingType<Type, OtherType>::IsPossible)
			{
				return false;
			}
			else
			{
				using CastType = typename HoldingType<Type, OtherType>::Type; // Find a type accomodating both types
				return (static_cast<CastType>(v) >= static_cast<CastType>(Min()) && static_cast<CastType>(v) <= static_cast<CastType>(Max()));
			}
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

	using UInt8 = RangedInteger<std::numeric_limits<uint8>::min(), std::numeric_limits<uint8>::max()>;
	using UInt16 = RangedInteger<std::numeric_limits<uint16>::min(), std::numeric_limits<uint16>::max()>;
	using UInt32 = RangedInteger<std::numeric_limits<uint32>::min(), std::numeric_limits<uint32>::max()>;
	using UInt64 = RangedInteger<std::numeric_limits<uint64>::min(), std::numeric_limits<uint64>::max()>;

	using Int8 = RangedInteger<std::numeric_limits<int8>::min(), std::numeric_limits<int8>::max()>;
	using Int16 = RangedInteger<std::numeric_limits<int16>::min(), std::numeric_limits<int16>::max()>;
	using Int32 = RangedInteger<std::numeric_limits<int32>::min(), std::numeric_limits<int32>::max()>;
	using Int64 = RangedInteger<std::numeric_limits<int64>::min(), std::numeric_limits<int64>::max()>;
}