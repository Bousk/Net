#include "Utils.hpp"
#include <numeric>

namespace Bousk
{
	namespace Utils
	{
		uint8 CountNeededBits(uint64 v)
		{
			assert(v != 0);
			uint8 bits = 0;
			while (v)
			{
				++bits;
				v /= 2;
			}
			return bits;
		}

		uint8 CreateRightBitsMask(uint8 rightBits)
		{
			assert(rightBits >= 1 && rightBits <= 8);
			switch (rightBits)
			{
			case 1: return 0b00000001;
			case 2: return 0b00000011;
			case 3: return 0b00000111;
			case 4: return 0b00001111;
			case 5: return 0b00011111;
			case 6: return 0b00111111;
			case 7: return 0b01111111;
			case 8: return 0b11111111;
			}
			return 0;
		}
		uint8 CreateBitsMask(uint8 nbBits, uint8 rightBitsToSkip)
		{
			assert(rightBitsToSkip < 8);
			assert(rightBitsToSkip + nbBits <= 8);
			return CreateRightBitsMask(nbBits) << rightBitsToSkip;
		}
		uint8 CreateLeftAlignedBitMask(uint8 nbBits, uint8 leftBitsToSkip)
		{
			assert(leftBitsToSkip < 8);
			assert(leftBitsToSkip + nbBits <= 8);
			return CreateRightBitsMask(nbBits) << (8 - leftBitsToSkip - nbBits);
		}
	}
}