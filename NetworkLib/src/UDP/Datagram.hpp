#pragma once

#include "Types.hpp"

#include <array>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			struct Datagram
			{
				using ID = uint16;
				enum class Type : uint8 {
					ConnectedData,
					KeepAlive,
				};
				struct Header
				{
					ID id;
					ID ack;
					uint64 previousAcks;
					Type type;
				};
				static constexpr uint16 BufferMaxSize = 1400;
				static constexpr uint16 HeaderSize = sizeof(Header);
				static constexpr uint16 DataMaxSize = BufferMaxSize - HeaderSize;

				Header header;
				std::array<uint8_t, DataMaxSize> data;

				// Not serialized
				size_t datasize{ 0 };
				//!< Datagram full size : Header + data
				size_t size() const { return HeaderSize + datasize; }
			};
		}
	}
}