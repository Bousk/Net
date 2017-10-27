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
			struct Header
			{
				ID id;
				ID ack;
				uint64_t previousAcks;
			};
			static constexpr size_t BufferMaxSize = 1400;
			static constexpr size_t DataMaxSize = BufferMaxSize - sizeof(Header);

			Header header;
			std::array<uint8_t, DataMaxSize> data;
		};
	}
}