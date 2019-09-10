#pragma once

#include "Serialization_Test.hpp"
#include "Tester.hpp"

#include <Serialization/Serializer.hpp>
#include <Serialization/Deserializer.hpp>

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
			Bousk::float32 value3;
			std::vector<Bousk::uint16> vector;
			std::string string;
			std::vector<std::string> vecStr;
			bool serialize(Bousk::Serialization::Serializer& serializer) const
			{
				return serializer.write(value1)
					&& serializer.write(value2)
					&& serializer.write(value3)
					&& serializer.write(vector)
					&& serializer.write(string)
					&& serializer.write(vecStr);
			}
			bool deserialize(Bousk::Serialization::Deserializer& deserializer)
			{
				return deserializer.read(value1)
					&& deserializer.read(value2)
					&& deserializer.read(value3)
					&& deserializer.read(vector)
					&& deserializer.read(string)
					&& deserializer.read(vecStr);
			}
			bool operator==(const MyFirstMessage& other) const
			{
				return value1 == other.value1
					&& value2 == other.value2
					&& std::abs(value3 - other.value3) <= std::numeric_limits<Bousk::float32>::epsilon()
					&& vector == other.vector
					&& string == other.string
					&& vecStr == other.vecStr;
			}
		};

		const MyFirstMessage origin_msg = { -37, 123456, 13.589f, {42, 349, 2895, 16578}, "toto va se baigner", {"toto", "mange", "un", "bonbon"} };

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
#undef READ_AND_CHECK
#undef READ_AND_CHECK_BOOL
	}
}