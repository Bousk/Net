#pragma once

#include "UDP/PacketHandling.hpp"
#include <cassert>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			void Multiplexer::queue(const unsigned char* data, const size_t datasize)
			{
				if (datasize > Packet::DataMaxSize)
				{
					//!< Split in even parts
					const auto parts = (datasize / Packet::DataMaxSize) + (((datasize % Packet::DataMaxSize) != 0) ? 1 : 0);
					const auto defaultFragmentSize = datasize / parts;
					size_t queuedSize = 0;
					for (unsigned i = 0; i < parts; ++i)
					{
						const bool isLastPart = (i == (parts - 1));
						const auto fragmentSize = static_cast<uint16_t>(isLastPart ? (datasize - parts * defaultFragmentSize) : defaultFragmentSize);
						Packet packet;
						packet.header.id = mNextId++;
						packet.header.type = ((i == 0) ? Packet::Type::FirstFragment : ((isLastPart) ? Packet::Type::LastFragment : Packet::Type::Fragment));
						packet.header.size = fragmentSize;
						memcpy(packet.data.data(), data, fragmentSize);
						mQueue.push_back(packet);
						queuedSize += fragmentSize;
					}
					assert(queuedSize == datasize);
				}
				else
				{
					//!< Single packet
					Packet packet;
					packet.header.id = mNextId++;
					packet.header.type = Packet::Type::Packet;
					packet.header.size = static_cast<uint16_t>(datasize);
					memcpy(packet.data.data(), data, datasize);
					mQueue.push_back(packet);
				}
			}
			size_t Multiplexer::serialize(unsigned char* buffer, const size_t buffersize)
			{
				size_t serializedSize = 0;
				for (auto packetit = mQueue.cbegin(); packetit != mQueue.cend();)
				{
					const auto& packet = *packetit;
					if (serializedSize + packet.size() > buffersize)
						break; // Not enough room for next packet

					memcpy(buffer, packet.data.data(), packet.size());
					serializedSize += packet.size();
					buffer += packet.size();

					// Once the packet has been serialized into a datagram, remove it fron the queue
					packetit = mQueue.erase(packetit);
				}
				return serializedSize;
			}
		}
	}
}