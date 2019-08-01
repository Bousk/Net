#include <Serialization/Deserializer.hpp>
#include <Serialization/Serialization.hpp>
#include <Serialization/Convert.hpp>
#include <Utils.hpp>

#include <algorithm>

namespace Bousk
{
	namespace Serialization
	{
		bool Deserializer::readBits(const uint8 nbBits, uint8* const buffer, const uint8 bufferSize)
		{
			assert(nbBits <= bufferSize * 8);
			if (remainingBits() < nbBits)
				return false;

			const size_t bufferBytesToWriteTo = (nbBits / 8) + (nbBits % 8 == 0 ? 0 : 1);
			// buffer here is in network/big endian, so bits must be write right (buffer + bufferBytesToWriteTo - 1) to left (buffer)
			for (uint8 totalReadBits = 0, writingBytesOffset = 1; totalReadBits < nbBits; ++writingBytesOffset)
			{
				uint8& dstByte = *(buffer + bufferBytesToWriteTo - writingBytesOffset);
				const uint8 bitsToRead = std::min(8, nbBits - totalReadBits);
				uint8 bitsRead = 0;
				{
					const uint8 srcByte = *(mBuffer + mBytesRead);
					// Read first bits from the current reading byte
					const uint8 remainingBitsInCurrentByte = 8 - mBitsRead;
					const uint8 leftBitsToSkip = mBitsRead;
					const uint8 bitsToReadFromCurrentByte = std::min(bitsToRead, remainingBitsInCurrentByte);
					const uint8 remainingBitsOnTheRight = 8 - bitsToReadFromCurrentByte - leftBitsToSkip;
					// Extract bits from the left
					const uint8 readMask = Utils::CreateBitsMask(bitsToReadFromCurrentByte, remainingBitsOnTheRight);
					const uint8 bits = srcByte & readMask;
					// Align those bits on the right in the output byte
					const uint8 bitsAlignedRight = bits >> remainingBitsOnTheRight;
					dstByte |= bitsAlignedRight;

					bitsRead += bitsToReadFromCurrentByte;
					mBitsRead += bitsToReadFromCurrentByte;
					mBytesRead += mBitsRead / 8;
					mBitsRead %= 8;
				}

				if (bitsRead < bitsToRead)
				{
					const uint8 srcByte = *(mBuffer + mBytesRead);
					// Read remaining bits of current output byte from next reading byte
					const uint8 bitsToReadFromCurrentByte = bitsToRead - bitsRead;
					const uint8 remainingBitsOnTheRight = 8 - bitsToReadFromCurrentByte;
					// Those bits are on the left
					const uint8 readMask = Utils::CreateBitsMask(bitsToReadFromCurrentByte, remainingBitsOnTheRight);
					const uint8 bits = srcByte & readMask;
					// Align them on the right to pack to first part read
					const uint8 bitsAlignedRightToPack = bits >> (8 - bitsToReadFromCurrentByte - bitsRead);
					dstByte |= bitsAlignedRightToPack;

					bitsRead += bitsToReadFromCurrentByte;
					mBitsRead += bitsToReadFromCurrentByte;
					mBytesRead += mBitsRead / 8;
					mBitsRead %= 8;
				}

				// Update counters
				totalReadBits += bitsRead;
			}

			return true;
		}

		bool Deserializer::read(uint8& data, uint8 minValue, uint8 maxValue)
		{
			assert(minValue < maxValue);
			const uint8 range = maxValue - minValue;
			uint8 bytesRead = 0;
			if (readBits(Utils::CountNeededBits(range), &bytesRead, 1))
			{
				data = bytesRead;
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
			const uint16 range = maxValue - minValue;
			uint8 bytesRead[2]{ 0 };
			if (readBits(Utils::CountNeededBits(range), bytesRead, 2))
			{
				Conversion::ToLocal(bytesRead, data);
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
			const uint32 range = maxValue - minValue;
			uint8 bytesRead[4]{ 0 };
			if (readBits(Utils::CountNeededBits(range), bytesRead, 4))
			{
				Conversion::ToLocal(bytesRead, data);
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
			static_assert(sizeof(int8) == sizeof(uint8), "");
			assert(minValue < maxValue);
			const uint8 range = static_cast<uint8>(maxValue - minValue);
			if (read(reinterpret_cast<uint8&>(data), 0, range))
			{
				data += minValue;
				return true;
			}
			return false;
		}
		bool Deserializer::read(int16& data, int16 minValue, int16 maxValue)
		{
			static_assert(sizeof(int16) == sizeof(uint16), "");
			assert(minValue < maxValue);
			const uint16 range = static_cast<uint16>(maxValue - minValue);
			if (read(reinterpret_cast<uint16&>(data), 0, range))
			{
				data += minValue;
				return true;
			}
			return false;
		}
		bool Deserializer::read(int32& data, int32 minValue, int32 maxValue)
		{
			static_assert(sizeof(int32) == sizeof(uint32), "");
			assert(minValue < maxValue);
			const uint32 range = static_cast<uint32>(maxValue - minValue);
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
			uint32 conv = 0;
			if (readBits(32, reinterpret_cast<uint8*>(&conv), 4))
			{
				Conversion::ToLocal(conv, data);
				return true;
			}
			return false;
		}
	}
}