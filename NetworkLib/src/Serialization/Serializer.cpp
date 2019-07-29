#include <Serialization/Serializer.hpp>
#include <Serialization/Convert.hpp>
#include <Serialization/Serialization.hpp>

namespace Bousk
{
	namespace Serialization
	{
		bool Serializer::writeBytes(const uint8* buffer, size_t nbBytes)
		{
			mBuffer.insert(mBuffer.cend(), buffer, buffer + nbBytes);
			return true;
		}

		bool Serializer::write(const uint8 data)
		{
			return writeBytes(&data, 1);
		}
		bool Serializer::write(const uint16 data)
		{
			uint16 conv;
			Conversion::ToNetwork(data, conv);
			return writeBytes(reinterpret_cast<const uint8*>(&conv), 2);
		}
		bool Serializer::write(const uint32 data)
		{
			uint32 conv;
			Conversion::ToNetwork(data, conv);
			return writeBytes(reinterpret_cast<const uint8*>(&conv), 4);
		}

		bool Serializer::write(const bool data)
		{
			return write(data ? BoolTrue : BoolFalse);
		}

		bool Serializer::write(const int8 data)
		{
			static_assert(sizeof(int8) == sizeof(uint8), "");
			return write(*reinterpret_cast<const uint8*>(&data));
		}
		bool Serializer::write(const int16 data)
		{
			static_assert(sizeof(int16) == sizeof(uint16), "");
			return write(*reinterpret_cast<const uint16*>(&data));
		}
		bool Serializer::write(const int32 data)
		{
			static_assert(sizeof(int32) == sizeof(uint32), "");
			return write(*reinterpret_cast<const uint32*>(&data));
		}

		bool Serializer::write(float32 data)
		{
			uint32 conv;
			Conversion::ToNetwork(data, conv);
			return writeBytes(reinterpret_cast<const uint8*>(&conv), 4);
		}

		bool Serializer::write(const std::string& data)
		{
			assert(data.size() <= std::numeric_limits<uint8>::max());
			return writeArray(reinterpret_cast<const uint8*>(data.data()), static_cast<uint8>(data.size()));
		}
	}
}