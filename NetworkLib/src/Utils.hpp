#pragma once

#include <cstdint>
#include <chrono>
#include <Types.hpp>

#define UNUSED(x) (void)(x)

namespace Bousk
{
	namespace Utils
	{
		inline std::chrono::milliseconds Now();

		inline bool IsSequenceNewer(uint16_t sNew, uint16_t sLast);
		inline uint16_t SequenceDiff(uint16_t sNew, uint16_t sLast);

		inline void SetBit(uint64_t& bitfield, uint8_t n);
		inline void UnsetBit(uint64_t& bitfield, uint8_t n);
		inline bool HasBit(uint64_t bitfield, uint8_t n);

		using Bousk::Bit;

		uint8 CountNeededBits(uint64 v);

		uint8 CreateRightBitsMask(uint8 rightBits);
		uint8 CreateBitsMask(uint8 nbBits, uint8 rightBitsToSkip);
	}
}

#include "Utils.inl"