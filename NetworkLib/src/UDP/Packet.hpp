#pragma once

#include <UDP/Datagram.hpp>
#include <UDP/ChannelHeader.hpp>
#include <Types.hpp>
#include <array>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			struct Packet
			{
				using Id = uint16;
				enum class Type : uint8
				{
					FullMessage,
					FirstFragment,
					Fragment,
					LastFragment,
				};
				struct Header
				{
					Id id;
					uint16 size{ 0 };
					Type type;
				};
				static constexpr uint16 PacketMaxSize = Datagram::DataMaxSize - ChannelHeader::Size;
				static constexpr uint16 HeaderSize = sizeof(Header);
				static constexpr uint16 DataMaxSize = PacketMaxSize - HeaderSize;
				static constexpr uint8 MaxPacketsPerMessage = 32;
				static constexpr uint16 MaxMessageSize = MaxPacketsPerMessage * DataMaxSize;

				Header header;
				std::array<uint8, DataMaxSize> dataBuffer;

				inline Id id() const { return header.id; }
				inline Type type() const { return header.type; }
				inline uint8* data() { return dataBuffer.data(); }
				inline const uint8* data() const { return dataBuffer.data(); }
				inline uint16 datasize() const { return header.size; }
				//!< Full buffer : header + data
				inline const uint8* buffer() const { return reinterpret_cast<const uint8*>(this); }
				//!< Packet full size : header + data
				inline uint16 size() const { return HeaderSize + header.size; }
			};
		}
	}
}