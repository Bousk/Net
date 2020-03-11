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
					Disconnection,
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
				std::array<uint8, DataMaxSize> data;

				// Not serialized
				uint16 datasize{ 0 };
				//!< Datagram full size : Header + data
				uint16 size() const { return HeaderSize + datasize; }
			};
		}
	}
}