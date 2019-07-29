#pragma once

#include "Tester.hpp"
#include "Serialization/Serializer.hpp"
#include "Serialization/Deserializer.hpp"
#include <vector>

class Serialization_Test
{
public:
	static void Test();
};

void Serialization_Test::Test()
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