#pragma once

#include <RangedInteger.hpp>
#include <Serialization/Serialization.hpp>
#include <Serialization/Serializer.hpp>
#include <Serialization/Deserializer.hpp>

namespace Bousk
{
	template<class FLOATTYPE, int32 MIN, int32 MAX, uint8 NBDECIMALS, uint8 STEP = 1 >
	class Float : public Serialization::Serializable
	{
		static_assert(std::is_same_v<FLOATTYPE, float32> || std::is_same_v<FLOATTYPE, float64>, "Float can only be used with float32 or float64");
		static_assert(NBDECIMALS > 0, "At least 1 decimal");
		static_assert(NBDECIMALS < 10, "Maximum 10 decimals");
		static_assert(STEP != 0, "Step must not be 0");
		static_assert(STEP % 10 != 0, "Step should not be a multiple of 10. Remove a decimal");
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
			return static_cast<uint32>(((value - Min) * Multiple) / Step);
		}

		inline FloatType get() const { return static_cast<FloatType>((mQuantizedValue.get() * Step * 1.) / Multiple + Min); }
		inline operator FloatType() const { return get(); }

		bool write(Serialization::Serializer& serializer) const override { return mQuantizedValue.write(serializer); }
		bool read(Serialization::Deserializer& deserializer) override { return mQuantizedValue.read(deserializer); }

	private:
		UInt32<0, Domain> mQuantizedValue;
	};
}