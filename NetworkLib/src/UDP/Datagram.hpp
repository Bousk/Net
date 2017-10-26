#pragma once

#include <cstdint>
#include <array>

namespace Bousk
{
	namespace UDP
	{
		struct Datagram
		{
			using ID = uint16_t;
			static constexpr uint8_t BufferMaxSize = 1400;
			static constexpr uint8_t DataMaxSize = BufferMaxSize - sizeof(ID);

			ID id;
			ID ack;
			uint64_t previousAcks;
			std::array<uint8_t, DataMaxSize> data;
		};
	}
}