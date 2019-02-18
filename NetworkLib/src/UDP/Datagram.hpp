#pragma once

#include <cstdint>
#include <array>

namespace Bousk
{
	namespace Network
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
				static constexpr uint16_t BufferMaxSize = 1400;
				static constexpr uint16_t HeaderSize = sizeof(Header);
				static constexpr uint16_t DataMaxSize = BufferMaxSize - HeaderSize;

				Header header;
				std::array<uint8_t, DataMaxSize> data;

				size_t datasize{ 0 };
				//!< Datagram full size : Header + data
				size_t size() const { return HeaderSize + datasize; }
			};
		}
	}
}