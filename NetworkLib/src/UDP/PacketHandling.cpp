#pragma once

#include "UDP/PacketHandling.hpp"
#include <cassert>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			void Multiplexer::queue(std::vector<unsigned char>&& data)
			{
				if (data.size() > Packet::DataMaxSize)
				{
					//!< Split in even parts
					const auto parts = (data.size() / Packet::DataMaxSize) + (((data.size() % Packet::DataMaxSize) != 0) ? 1 : 0);
					const auto defaultFragmentSize = data.size() / parts;
					size_t queuedSize = 0;
					for (unsigned i = 0; i < parts; ++i)
					{
						const bool isLastPart = (i == (parts - 1));
						const auto fragmentSize = static_cast<uint16_t>(isLastPart ? (data.size() - parts * defaultFragmentSize) : defaultFragmentSize);
						Packet packet;
						packet.header.id = mNextId++;
						packet.header.type = ((i == 0) ? Packet::Type::FirstFragment : ((isLastPart) ? Packet::Type::LastFragment : Packet::Type::Fragment));
						packet.header.size = fragmentSize;
						memcpy(packet.data(), data.data() + queuedSize, fragmentSize);
						mQueue.push_back(packet);
						queuedSize += fragmentSize;
					}
					assert(queuedSize == data.size());
				}
				else
				{
					//!< Single packet
					Packet packet;
					packet.header.id = mNextId++;
					packet.header.type = Packet::Type::Packet;
					packet.header.size = static_cast<uint16_t>(data.size());
					memcpy(packet.data(), data.data(), data.size());
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

					memcpy(buffer, packet.data(), packet.size());
					serializedSize += packet.size();
					buffer += packet.size();

					// Once the packet has been serialized into a datagram, remove it fron the queue
					packetit = mQueue.erase(packetit);
				}
				return serializedSize;
			}
			void Demultiplexer::queue(const Packet& pckt)
			{}
			std::vector<Packet> Demultiplexer::process()
			{
				return std::vector<Packet>();
			}
		}
	}
}