#pragma once

#include <Types.hpp>
#include <chrono>

#define UNUSED(x) (void)(x)

namespace Bousk
{
	namespace Utils
	{
		inline std::chrono::milliseconds Now();

		inline bool IsSequenceNewer(uint16 sNew, uint16 sLast);
		inline uint16 SequenceDiff(uint16 sNew, uint16 sLast);

		inline void SetBit(uint64& bitfield, uint8 n);
		inline void UnsetBit(uint64& bitfield, uint8 n);
		inline bool HasBit(uint64 bitfield, uint8 n);

		using Bousk::Bit;

		uint8 CountNeededBits(uint64 v);

		uint8 CreateRightBitsMask(uint8 rightBits);
		uint8 CreateBitsMask(uint8 nbBits, uint8 rightBitsToSkip);
	}
}

#include "Utils.inl"