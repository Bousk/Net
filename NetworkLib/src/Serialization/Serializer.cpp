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
				uint8 writtenBits = 0;
				if (mUsedBits)
				{
					// We have an existing byte to pack data to
					const uint8 remainingBitsInCurrentByte = 8 - mUsedBits;
					const uint8 nbBitsToPack = std::min(bitsToWrite, remainingBitsInCurrentByte);
					// Extract bits from the right
					const uint8 rightBitsToPack = srcByte & Utils::CreateRightBitsMask(nbBitsToPack);
					// Align those bits on the left to pack to existing bits in buffer
					const uint8 bitsShiftToAlignLeft = remainingBitsInCurrentByte - nbBitsToPack;
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
					const uint8 bitsShiftToAlignRight = writtenBits;
					const uint8 rightAlignedBits = leftBitsToPack >> bitsShiftToAlignRight;
					// Align bits to the left on the new byte
					const uint8 bitsShiftToAlignLeft = 8 - remainingBits;
					const uint8 leftAlignedBits = rightAlignedBits << bitsShiftToAlignLeft;
					// Add those bits as a new byte to the buffer, they are aligned on the left in new byte
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
			assert(minValue < maxValue);
			assert(minValue <= data && data <= maxValue);
			const uint8 rangedData = data - minValue;
			const uint8 range = maxValue - minValue;
			return writeBits(&rangedData, 1, Utils::CountNeededBits(range));
		}
		bool Serializer::write(uint16 data, uint16 minValue, uint16 maxValue)
		{
			assert(minValue < maxValue);
			assert(minValue <= data && data <= maxValue);
			const uint16 rangedData = data - minValue;
			const uint16 range = maxValue - minValue;
			uint16 conv;
			Conversion::ToNetwork(rangedData, conv);
			return writeBits(reinterpret_cast<const uint8*>(&conv), 2, Utils::CountNeededBits(range));
		}
		bool Serializer::write(uint32 data, uint32 minValue, uint32 maxValue)
		{
			assert(minValue < maxValue);
			assert(minValue <= data && data <= maxValue);
			const uint32 rangedData = data - minValue;
			const uint32 range = maxValue - minValue;
			uint32 conv;
			Conversion::ToNetwork(rangedData, conv);
			return writeBits(reinterpret_cast<const uint8*>(&conv), 4, Utils::CountNeededBits(range));
		}

		bool Serializer::write(const int8 data, int8 minValue, int8 maxValue)
		{
			static_assert(sizeof(int8) == sizeof(uint8), "");
			assert(minValue < maxValue);
			assert(minValue <= data && data <= maxValue);
			const uint8 rangedData = static_cast<uint8>(data - minValue);
			const uint8 range = static_cast<uint8>(maxValue - minValue);
			return write(*reinterpret_cast<const uint8*>(&rangedData), static_cast<uint8>(0), range);
		}
		bool Serializer::write(const int16 data, int16 minValue, int16 maxValue)
		{
			static_assert(sizeof(int16) == sizeof(uint16), "");
			assert(minValue < maxValue);
			assert(minValue <= data && data <= maxValue);
			const uint16 rangedData = static_cast<uint16>(data - minValue);
			const uint16 range = static_cast<uint16>(maxValue - minValue);
			return write(*reinterpret_cast<const uint16*>(&rangedData), static_cast<uint16>(0), range);
		}
		bool Serializer::write(const int32 data, int32 minValue, int32 maxValue)
		{
			static_assert(sizeof(int32) == sizeof(uint32), "");
			assert(minValue < maxValue);
			assert(minValue <= data && data <= maxValue);
			const uint32 rangedData = static_cast<uint32>(data - minValue);
			const uint32 range = static_cast<uint32>(maxValue - minValue);
			return write(*reinterpret_cast<const uint32*>(&rangedData), static_cast<uint32>(0), range);
		}

		bool Serializer::write(float32 data)
		{
			uint32 conv;
			Conversion::ToNetwork(data, conv);
			return writeBits(reinterpret_cast<const uint8*>(&conv), 4, 32);
		}
	}
}