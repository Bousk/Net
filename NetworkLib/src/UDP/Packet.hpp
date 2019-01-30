#pragma once

#include "UDP/Datagram.hpp"
#include <cstdint>
#include <array>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			struct Packet
			{
				using Id = uint16_t;
				enum class Type : uint8_t
				{
					Packet,
					FirstFragment,
					Fragment,
					LastFragment,
				};
				struct Header
				{
					Id id;
					uint16_t size;
					Type type;
				};
				static constexpr uint16_t PacketMaxSize = Datagram::DataMaxSize;
				static constexpr uint16_t HeaderSize = sizeof(Header);
				static constexpr uint16_t DataMaxSize = PacketMaxSize - HeaderSize;

				Header header;
				std::array<uint8_t, DataMaxSize> dataBuffer;

				Id id() const { return header.id; }
				Type type() const { return header.type; }
				uint8_t* data() { return dataBuffer.data(); }
				const uint8_t* data() const { return dataBuffer.data(); }
				inline uint16_t datasize() const { return header.size; }
				// Full buffer : header + data
				inline const uint8_t* buffer() const { return reinterpret_cast<const uint8_t*>(this); }
				// Packet full size : header + data
				inline uint16_t size() const { return HeaderSize + header.size; }
			};
		}
	}
}