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


		bool Deserializer::read(uint8& data)
		{
			return readBytes(1, &data);
		}
		bool Deserializer::read(uint16& data)
		{
			uint8 bytesRead[2];
			if (!readBytes(2, bytesRead))
				return false;
			Conversion::ToLocal(bytesRead, data);
			return true;
		}
		bool Deserializer::read(uint32& data)
		{
			uint8 bytesRead[4];
			if (!readBytes(4, bytesRead))
				return false;
			Conversion::ToLocal(bytesRead, data);
			return true;
		}

		bool Deserializer::read(bool& data)
		{
			uint8 byteRead;
			if (!readBytes(1, &byteRead))
				return false;
			data = (byteRead == BoolTrue);
			return true;
		}

		bool Deserializer::read(int8& data)
		{
			return read(reinterpret_cast<uint8&>(data));
		}
		bool Deserializer::read(int16& data)
		{
			return read(reinterpret_cast<uint16&>(data));
		}
		bool Deserializer::read(int32& data)
		{
			return read(reinterpret_cast<uint32&>(data));
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