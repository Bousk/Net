#pragma once

#include <cstdint>
#include <chrono>

#define UNUSED(x) (void)(x)

namespace Bousk
{
	namespace Utils
	{
		inline std::chrono::milliseconds Now();

		inline bool IsSequenceNewer(const uint16_t sNew, const uint16_t sLast);
		inline uint16_t SequenceDiff(const uint16_t sNew, const uint16_t sLast);

		inline void SetBit(uint64_t& bitfield, const uint8_t n);
		inline void UnsetBit(uint64_t& bitfield, const uint8_t n);
		inline bool HasBit(uint64_t bitfield, const uint8_t n);

		template<class INTEGER>
		struct Bit {};
		template<>
		struct Bit<uint64_t> {
			static constexpr uint64_t Right = 0b0000000000000000000000000000000000000000000000000000000000000001;
		};
	}
}

#include "Utils.inl"