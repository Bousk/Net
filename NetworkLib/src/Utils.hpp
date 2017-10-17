#pragma once

#include <cstdint>
#include <chrono>

#define UNUSED(x) (void)(x)

namespace Bousk
{
	namespace Utils
	{
		std::chrono::milliseconds Now();

		bool IsSequenceNewer(uint32_t sNew, uint32_t sLast);
		uint32_t SequenceDiff(uint32_t sNew, uint32_t sLast);
	}
}