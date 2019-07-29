#include <Serialization/Serializer.hpp>
#include <Serialization/Convert.hpp>
#include <Serialization/Serialization.hpp>
#include <Utils.hpp>

#include <algorithm>

namespace Bousk
{
	namespace Serialization
	{
		bool Serializer::writeBits(const uint8* buffer, const uint8 buffersize, const uint8 nbBits)
		{
			// buffer here is in network/big endian, so bits to write must be read right (buffer + buffersize - 1) to left (buffer)
			for (uint8 totalWrittenBits = 0, readingBytesOffset = 1; totalWrittenBits < nbBits; ++readingBytesOffset)
			{
				const uint8 srcByte = *(buffer + buffersize - readingBytesOffset);
				const uint8 bitsToWrite = std::min(8, nbBits - totalWrittenBits);
				const uint8 availableBits = (8 - mUsedBits) % 8;
				uint8 writtenBits = 0;
				if (availableBits)
				{
					// We have an existing byte to pack data to
					const uint8 nbBitsToPack = std::min(bitsToWrite, availableBits);
					// Extract bits from the right
					const uint8 rightBitsToPack = srcByte & Utils::CreateRightBitsMask(nbBitsToPack);
					// Align those bits on the left to pack to existing bits in buffer
					const uint8 bitsShiftToAlignLeft = availableBits - nbBitsToPack;
					const uint8 leftAlignedBits = rightBitsToPack << bitsShiftToAlignLeft;
					mBuffer.back() |= leftAlignedBits;
					writtenBits += nbBitsToPack;
				}
				const uint8 remainingBits = bitsToWrite - writtenBits;
				if (remainingBits)
				{
					// Extract bits to serialize
					const uint8 leftBitsToPack = srcByte & Utils::CreateBitsMask(remainingBits, writtenBits);
					// Remove bits on the right : align the interesting ones on the right
					const uint8 bitsShiftToAlignRight = 8 - writtenBits - remainingBits;
					const uint8 rightAlignedBits = leftBitsToPack >> bitsShiftToAlignRight;
					// Align bits to the left on the new byte
					const uint8 bitsShiftToAlignLeft = 8 - remainingBits;
					const uint8 leftAlignedBits = rightAlignedBits << bitsShiftToAlignLeft;
					// Add those bits as a new byte to the buffer
					mBuffer.push_back(leftAlignedBits);
					writtenBits += remainingBits;
				}
				// Update our counters
				totalWrittenBits += writtenBits;
				mUsedBits += writtenBits;
				mUsedBits %= 8;
			}
			return true;
		}

		bool Serializer::write(uint8 data, uint8 minValue, uint8 maxValue)
		{
			return writeBits(&data, 1, Utils::CountNeededBits(maxValue - minValue));
		}
		bool Serializer::write(uint16 data, uint16 minValue, uint16 maxValue)
		{
			uint16 conv;
			Conversion::ToNetwork(data, conv);
			return writeBits(reinterpret_cast<const uint8*>(&conv), 2, Utils::CountNeededBits(maxValue - minValue));
		}
		bool Serializer::write(uint32 data, uint32 minValue, uint32 maxValue)
		{
			uint32 conv;
			Conversion::ToNetwork(data, conv);
			return writeBits(reinterpret_cast<const uint8*>(&conv), 4, Utils::CountNeededBits(maxValue - minValue));
		}

		bool Serializer::writeBit(bool data)
		{
			if (mUsedBits && data)
			{
				const uint8 bitsShiftToAlignLeft = 8 - mUsedBits;
				const uint8 leftAlignedBit = Bit<uint8>::Right << bitsShiftToAlignLeft;
				mBuffer.back() |= leftAlignedBit;
			}
			else if (data)
			{
				mBuffer.push_back(Bit<uint8>::Right);
			}
			++mUsedBits;
			mUsedBits %= 8;
			return true;
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
			return writeBits(reinterpret_cast<const uint8*>(&conv), 4, 32);
		}
	}
}