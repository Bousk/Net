#pragma once

#include "Serialization_Test.hpp"
#include "Tester.hpp"

#include <Serialization/Serializer.hpp>
#include <Serialization/Deserializer.hpp>
#include <RangedInteger.hpp>
#include <Float.hpp>

#include <string>
#include <vector>

void Serialization_Test::TestBasics()
{
	{
		Bousk::Serialization::Serializer serializer;

		const Bousk::int8 origin_i8 = 0x12;
		const Bousk::int16 origin_i16 = 0x3456;
		const Bousk::int32 origin_i32 = 0x789ABCDE;

		CHECK(serializer.write(origin_i8));
		CHECK(serializer.write(origin_i16));
		CHECK(serializer.write(origin_i32));
		CHECK(serializer.bufferSize() == 7);
		Bousk::Serialization::Deserializer deserializer(serializer.buffer(), serializer.bufferSize());
		CHECK(deserializer.remainingBytes() == 7);
		Bousk::int8 i8;
		CHECK(deserializer.read(i8));
		CHECK(i8 == origin_i8);
		Bousk::int16 i16;
		CHECK(deserializer.read(i16));
		CHECK(i16 == origin_i16);
		Bousk::int32 i32;
		CHECK(deserializer.read(i32));
		CHECK(i32 == origin_i32);
		CHECK(deserializer.remainingBytes() == 0);
	}
	{
		struct MyFirstMessage
		{
			Bousk::int8 value1;
			Bousk::uint32 value2;
		#if BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
			Bousk::float32 value3;
		#endif // BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
			std::vector<Bousk::uint16> vector;
			std::string string;
			std::vector<std::string> vecStr;
			bool serialize(Bousk::Serialization::Serializer& serializer) const
			{
				return serializer.write(value1)
					&& serializer.write(value2)
				#if BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
					&& serializer.write(value3)
				#endif // BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
					&& serializer.write(vector)
					&& serializer.write(string)
					&& serializer.write(vecStr);
			}
			bool deserialize(Bousk::Serialization::Deserializer& deserializer)
			{
				return deserializer.read(value1)
					&& deserializer.read(value2)
				#if BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
					&& deserializer.read(value3)
				#endif // BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
					&& deserializer.read(vector)
					&& deserializer.read(string)
					&& deserializer.read(vecStr);
			}
			bool operator==(const MyFirstMessage& other) const
			{
				return value1 == other.value1
					&& value2 == other.value2
				#if BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
					&& std::abs(value3 - other.value3) <= std::numeric_limits<Bousk::float32>::epsilon()
				#endif // BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
					&& vector == other.vector
					&& string == other.string
					&& vecStr == other.vecStr;
			}
		};

		const MyFirstMessage origin_msg = { -37, 123456
		#if BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
			, 13.589f
		#endif // BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
			, {42, 349, 2895, 16578}, "toto va se baigner", {"toto", "mange", "un", "bonbon"} };

		Bousk::Serialization::Serializer serializer;
		CHECK(origin_msg.serialize(serializer));

		MyFirstMessage msg;
		Bousk::Serialization::Deserializer deserializer(serializer.buffer(), serializer.bufferSize());
		CHECK(msg.deserialize(deserializer));

		CHECK(origin_msg == msg);
	}
}

void Serialization_Test::TestBits()
{
	{
		Bousk::Serialization::Serializer serializer;
		
#define WRITE_AS(TYPE, VALUE, RANGE_MIN, RANGE_MAX) serializer.write(static_cast<TYPE>(VALUE), static_cast<TYPE>(RANGE_MIN), static_cast<TYPE>(RANGE_MAX))
		// Write a full byte
		CHECK(WRITE_AS(Bousk::uint8, 5, 0, 255));
		CHECK(serializer.bufferSize() == 1);
		CHECK(serializer.mUsedBits == 0);
		CHECK(serializer.mBuffer[0] == 0b00000101);
		// Write 4 bits
		CHECK(WRITE_AS(Bousk::int8, 3, -7, 8)); // 3 on [-7, 8] => 10 on [0, 15] => 1010
		CHECK(serializer.bufferSize() == 2);
		CHECK(serializer.mUsedBits == 4);
		CHECK(serializer.mBuffer[0] == 0b00000101);
		CHECK(serializer.mBuffer[1] == 0b10100000);
		// Write 5 bits
		CHECK(WRITE_AS(Bousk::uint8, 18, 0, 31)); // => 1 0010
		CHECK(serializer.bufferSize() == 3);
		CHECK(serializer.mUsedBits == 1);
		CHECK(serializer.mBuffer[0] == 0b00000101);
		CHECK(serializer.mBuffer[1] == 0b10100010);
		CHECK(serializer.mBuffer[2] == 0b10000000);
		// Write a couple bools
		CHECK(serializer.write(true));
		CHECK(serializer.bufferSize() == 3);
		CHECK(serializer.mUsedBits == 2);
		CHECK(serializer.mBuffer[0] == 0b00000101);
		CHECK(serializer.mBuffer[1] == 0b10100010);
		CHECK(serializer.mBuffer[2] == 0b11000000);
		CHECK(serializer.write(false));
		CHECK(serializer.bufferSize() == 3);
		CHECK(serializer.mUsedBits == 3);
		CHECK(serializer.mBuffer[0] == 0b00000101);
		CHECK(serializer.mBuffer[1] == 0b10100010);
		CHECK(serializer.mBuffer[2] == 0b11000000);
		// Pack a 13 bits value
		CHECK(WRITE_AS(Bousk::int16, 3578, -4000, 4000)); // 7578 [0, 8000] => 11101 100 11010
		CHECK(serializer.bufferSize() == 4);
		CHECK(serializer.mUsedBits == 0);
		CHECK(serializer.mBuffer[0] == 0b00000101);
		CHECK(serializer.mBuffer[1] == 0b10100010);
		CHECK(serializer.mBuffer[2] == 0b11011010);
		CHECK(serializer.mBuffer[3] == 0b10011101);
		// Write a 9 bits uint32
		CHECK(WRITE_AS(Bousk::uint32, 123, 0, 256));
		CHECK(serializer.bufferSize() == 6);
		CHECK(serializer.mUsedBits == 1);
		CHECK(serializer.mBuffer[0] == 0b00000101);
		CHECK(serializer.mBuffer[1] == 0b10100010);
		CHECK(serializer.mBuffer[2] == 0b11011010);
		CHECK(serializer.mBuffer[3] == 0b10011101);
		CHECK(serializer.mBuffer[4] == 0b01111011);
		CHECK(serializer.mBuffer[5] == 0b00000000);
#undef WRITE_AS

		Bousk::Serialization::Deserializer deserializer(serializer.buffer(), serializer.bufferSize());
#define READ_AND_CHECK(TYPE, EXPECTED_VALUE, RANGE_MIN, RANGE_MAX)										\
		do {																							\
			TYPE data;																					\
			CHECK(deserializer.read(data, static_cast<TYPE>(RANGE_MIN), static_cast<TYPE>(RANGE_MAX)));	\
			CHECK(data == static_cast<TYPE>(EXPECTED_VALUE));								\
		} while(0)
#define READ_AND_CHECK_BOOL(EXPECTED_VALUE)	\
		do {								\
			bool data;						\
			CHECK(deserializer.read(data));	\
			CHECK(data == EXPECTED_VALUE);	\
		} while(0)
		// Full byte
		READ_AND_CHECK(Bousk::uint8, 5, 0, 255);
		CHECK(deserializer.bufferReadBits() == 8);
		// 4 bits
		READ_AND_CHECK(Bousk::int8, 3, -7, 8);
		CHECK(deserializer.mBytesRead == 1);
		CHECK(deserializer.mBitsRead == 4);
		CHECK(deserializer.bufferReadBits() == 12);
		// 5 bits
		READ_AND_CHECK(Bousk::uint8, 18, 0, 31);
		CHECK(deserializer.mBytesRead == 2);
		CHECK(deserializer.mBitsRead == 1);
		CHECK(deserializer.bufferReadBits() == 17);
		// Read a couple bools
		READ_AND_CHECK_BOOL(true);
		CHECK(deserializer.mBytesRead == 2);
		CHECK(deserializer.mBitsRead == 2);
		CHECK(deserializer.bufferReadBits() == 18);
		READ_AND_CHECK_BOOL(false);
		CHECK(deserializer.mBytesRead == 2);
		CHECK(deserializer.mBitsRead == 3);
		CHECK(deserializer.bufferReadBits() == 19);
		// 13 bits value
		READ_AND_CHECK(Bousk::int16, 3578, -4000, 4000);
		CHECK(deserializer.mBytesRead == 4);
		CHECK(deserializer.mBitsRead == 0);
		CHECK(deserializer.bufferReadBits() == 32);
		// 9 bits uint32
		READ_AND_CHECK(Bousk::int32, 123, 0, 256);
		CHECK(deserializer.mBytesRead == 5);
		CHECK(deserializer.mBitsRead == 1);
		CHECK(deserializer.bufferReadBits() == 41);
#undef READ_AND_CHECK
#undef READ_AND_CHECK_BOOL
	}
}

void Serialization_Test::TestAdvanced()
{
	STATIC_CHECK(std::is_same_v<Bousk::Biggest<Bousk::uint8, Bousk::uint8>::Type, Bousk::uint8>);
	STATIC_CHECK(std::is_same_v<Bousk::Biggest<Bousk::uint8, Bousk::uint32>::Type, Bousk::uint32>);
	STATIC_CHECK(std::is_same_v<Bousk::Biggest<Bousk::int16, Bousk::uint64>::Type, Bousk::uint64>);
	STATIC_CHECK(std::is_same_v<Bousk::HoldingType<Bousk::uint8, Bousk::uint8>::Type, Bousk::uint8>);
	STATIC_CHECK(std::is_same_v<Bousk::HoldingType<Bousk::uint8, Bousk::uint16>::Type, Bousk::uint16>);
	STATIC_CHECK(std::is_same_v<Bousk::HoldingType<Bousk::uint8, Bousk::int16>::Type, Bousk::int16>);
	STATIC_CHECK(std::is_same_v<Bousk::Biggest<Bousk::int8, Bousk::uint16>::Type, Bousk::uint16>);
	STATIC_CHECK(std::is_same_v<Bousk::HoldingType<Bousk::int8, Bousk::uint16>::Type, Bousk::int32>);
	STATIC_CHECK(std::is_same_v<Bousk::ExtractType<1, false>::Type, Bousk::int8>);
	STATIC_CHECK(std::is_same_v<Bousk::ExtractType<1, true>::Type, Bousk::uint8>);
	STATIC_CHECK(std::is_same_v<Bousk::ExtractType<200, false>::Type, Bousk::int16>);
	STATIC_CHECK(std::is_same_v<Bousk::ExtractType<200, true>::Type, Bousk::uint8>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<-1, 1>::MinType, Bousk::int8>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<-1, 1>::MaxType, Bousk::int8>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<-1, 1>::Type, Bousk::int8>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<-1, 200>::MinType, Bousk::int8>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<-1, 200>::MaxType, Bousk::int16>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<-1, 200>::Type, Bousk::int16>);
	STATIC_CHECK(std::is_same_v<Bousk::ExtractType<std::numeric_limits<Bousk::int32>::min(), false>::Type, Bousk::int32>);
	STATIC_CHECK(std::is_same_v<Bousk::ExtractType<std::numeric_limits<Bousk::int64>::max(), false>::Type, Bousk::int64>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<std::numeric_limits<Bousk::int32>::min(), std::numeric_limits<Bousk::int32>::max()>::Type, Bousk::int32>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<std::numeric_limits<Bousk::int32>::min(), std::numeric_limits<Bousk::int64>::max()>::Type, Bousk::int64>);
	STATIC_CHECK(std::is_same_v<Bousk::RangedInteger<std::numeric_limits<Bousk::int32>::min(), std::numeric_limits<Bousk::int64>::max()>::Type, Bousk::int64>);
	STATIC_CHECK(std::is_same_v<Bousk::RangedInteger<std::numeric_limits<Bousk::int64>::min(), std::numeric_limits<Bousk::int64>::max()>::Type, Bousk::int64>);
	STATIC_CHECK(Bousk::RangedInteger<std::numeric_limits<Bousk::int32>::min(), std::numeric_limits<Bousk::int64>::max()>::IsWithinRange(42));
	STATIC_CHECK(Bousk::RangedInteger<std::numeric_limits<Bousk::int64>::min(), std::numeric_limits<Bousk::int64>::max()>::IsWithinRange(42));
	STATIC_CHECK(!Bousk::RangedInteger<std::numeric_limits<Bousk::int64>::min(), std::numeric_limits<Bousk::int64>::max()>::IsWithinRange(std::numeric_limits<Bousk::uint64>::max()));
	STATIC_CHECK(std::is_same_v<Bousk::HoldingType<Bousk::int64, Bousk::uint64>::Type, void>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<std::numeric_limits<Bousk::int64>::min(), std::numeric_limits<Bousk::uint64>::max()>::MinType, Bousk::int64>);
	STATIC_CHECK(std::is_same_v<Bousk::FittingType<std::numeric_limits<Bousk::int64>::min(), std::numeric_limits<Bousk::uint64>::max()>::MaxType, Bousk::uint64>);
	STATIC_CHECK(!Bousk::FittingType<std::numeric_limits<Bousk::int64>::min(), std::numeric_limits<Bousk::uint64>::max()>::IsPossible);
	//Bousk::RangedInteger<std::numeric_limits<Bousk::int64>::min(), std::numeric_limits<Bousk::uint64>::max()> impossibleVar;
	{
		enum Enum1 { Min, Entry1 = Min, Entry2, Entry3, Entry4, Max = Entry4 };
		enum class Enum2 : Bousk::uint8 { Min, Entry1 = Min, Entry2, Entry3, Entry4, Max = Entry4 };
		Bousk::Serialization::Serializer serializer;
		std::vector<Bousk::RangedInteger<0, 42>> vec{ 0, 2, 4, 8, 16, 32 };
		Bousk::Float<Bousk::float32, -5, 5, 3> fv = -2.048f;
		CHECK(serializer.write(vec));
		CHECK(serializer.write(fv));
		CHECK(serializer.write(Enum1::Entry1));
		CHECK(serializer.write(Enum2::Entry3));

		Bousk::Serialization::Deserializer deserializer(serializer.buffer(), serializer.bufferSize());
		std::vector<Bousk::RangedInteger<0, 42>> vec2;
		CHECK(deserializer.read(vec2));
		CHECK(vec == vec2);
		Bousk::Float<Bousk::float32, -5, 5, 3> fv2;
		CHECK(deserializer.read(fv2));
		CHECK(fv == fv2);
		Enum1 e1;
		CHECK(deserializer.read(e1));
		CHECK(e1 == Enum1::Entry1);
		Enum2 e2;
		CHECK(deserializer.read(e2));
		CHECK(e2 == Enum2::Entry3);
	}
}
