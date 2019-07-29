#include <Serialization/Deserializer.hpp>
#include <Serialization/Serialization.hpp>
#include <Serialization/Convert.hpp>

namespace Bousk
{
	namespace Serialization
	{
		bool Deserializer::readBytes(size_t nbBytes, uint8* buffer)
		{
			if (remainingBytes() < nbBytes)
				return false;

			for (size_t i = 0; i < nbBytes; ++i)
				buffer[i] = mBuffer[mBytesRead + i];

			mBytesRead += nbBytes;
			return true;
		}

		bool Deserializer::read(uint8& data, uint8 minValue, uint8 maxValue)
		{
			assert(minValue < maxValue);
			if (readBytes(1, &data))
			{
				const uint8 range = maxValue - minValue;
				if (data <= range)
				{
					data += minValue;
					return true;
				}
			}
			return false;
		}
		bool Deserializer::read(uint16& data, uint16 minValue, uint16 maxValue)
		{
			assert(minValue < maxValue);
			uint8 bytesRead[2];
			if (readBytes(2, bytesRead))
			{
				Conversion::ToLocal(bytesRead, data);
				const uint16 range = maxValue - minValue;
				if (data <= range)
				{
					data += minValue;
					return true;
				}
			}
			return false;
		}
		bool Deserializer::read(uint32& data, uint32 minValue, uint32 maxValue)
		{
			assert(minValue < maxValue);
			uint8 bytesRead[4];
			if (!readBytes(4, bytesRead))
			{
				Conversion::ToLocal(bytesRead, data);
				const uint16 range = maxValue - minValue;
				if (data <= range)
				{
					data += minValue;
					return true;
				}
			}
			return false;
		}

		bool Deserializer::read(int8& data, int8 minValue, int8 maxValue)
		{
			assert(minValue < maxValue);
			assert(minValue <= data && data <= maxValue);
			const uint8 range = static_cast<uint8>(maxValue - minValue);
			static_assert(sizeof(int8) == sizeof(uint8), "");
			if (read(reinterpret_cast<uint8&>(data), 0, range))
			{
				data += minValue;
				return true;
			}
			return false;
		}
		bool Deserializer::read(int16& data, int16 minValue, int16 maxValue)
		{
			assert(minValue < maxValue);
			assert(minValue <= data && data <= maxValue);
			const uint16 range = static_cast<uint16>(maxValue - minValue);
			static_assert(sizeof(int16) == sizeof(uint16), "");
			if (read(reinterpret_cast<uint16&>(data), 0, range))
			{
				data += minValue;
				return true;
			}
			return false;
		}
		bool Deserializer::read(int32& data, int32 minValue, int32 maxValue)
		{
			assert(minValue < maxValue);
			assert(minValue <= data && data <= maxValue);
			const uint32 range = static_cast<uint32>(maxValue - minValue);
			static_assert(sizeof(int32) == sizeof(uint32), "");
			if (read(reinterpret_cast<uint32&>(data), 0, range))
			{
				data += minValue;
				return true;
			}
			return false;
		}

		bool Deserializer::read(bool& data)
		{
			uint8 byteRead;
			if (read(byteRead, 0, 1))
			{
				data = (byteRead == BoolTrue);
				return true;
			}
			return false;
		}

		bool Deserializer::read(float32& data)
		{
			uint32 conv;
			if (readBytes(4, reinterpret_cast<uint8*>(&conv)))
			{
				Conversion::ToLocal(conv, data);
				return true;
			}
			return false;
		}
	}
}