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
				static constexpr size_t PacketMaxSize = Datagram::DataMaxSize;
				static constexpr size_t DataMaxSize = PacketMaxSize - sizeof(Header);

				Header header;
				std::array<uint8_t, DataMaxSize> buffer;

				Id id() const { return header.id; }
				Type type() const { return header.type; }
				uint8_t* data() { return buffer.data(); }
				const uint8_t* data() const { return buffer.data(); }
				inline uint16_t size() const { return header.size; }
			};
		}
	}
}