#pragma once

#include "UDP/PacketHandling.hpp"
#include <cassert>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			void Multiplexer::queue(std::vector<uint8_t>&& data)
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
			size_t Multiplexer::serialize(uint8_t* buffer, const size_t buffersize)
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
			void Demultiplexer::onDataReceived(const uint8_t* data, const size_t datasize)
			{
				// Extract packets from buffer
				size_t processedDataSize = 0;
				while (processedDataSize < datasize)
				{
					const Packet* pckt = reinterpret_cast<const Packet*>(data);
					if (processedDataSize + pckt->size() > datasize || pckt->size() > Packet::DataMaxSize)
					{
						// Malformed packet or buffer
						return;
					}
					onPacketReceived(pckt);
					processedDataSize += pckt->size();
					data += pckt->size();
				}
			}
			void Demultiplexer::onPacketReceived(const Packet* pckt)
			{
				if (pckt->id() <= mLastProcessed)
					return; // Packet is too old

				// Find the place for this packet, our queue must remain ordered
				if (mPendingQueue.empty() || pckt->id() > mPendingQueue.back().id())
				{
					mPendingQueue.push_back(*pckt);
				}
				else
				{
					// Find the first iterator with an id bigger than our packet, we must place the packet before that one
					auto insertLocation = std::find_if(mPendingQueue.cbegin(), mPendingQueue.cend(), [&pckt](const Packet& p) { return p.id() >= pckt->id(); });
					// Make sure we don't insert a packet received multiple times
					if (insertLocation->id() != pckt->id())
					{
						mPendingQueue.insert(insertLocation, *pckt);
					}
				}
			}
			std::vector<std::vector<uint8_t>> Demultiplexer::process()
			{
				std::vector<std::vector<uint8_t>> messagesReady;

				auto itPacket = mPendingQueue.cbegin();
				auto itEnd = mPendingQueue.cend();
				std::vector<Packet>::const_iterator newestProcessedPacket;
				// Our queue is ordered, just go through and reassemble packets
				while(itPacket != itEnd)
				{
					if (itPacket->type() == Packet::Type::Packet)
					{
						// Full packet, just take it
						std::vector<uint8_t> msg(itPacket->data(), itPacket->data() + itPacket->size());
						messagesReady.push_back(std::move(msg));
						newestProcessedPacket = itPacket;
						++itPacket;
					}
					else if (itPacket->type() == Packet::Type::FirstFragment)
					{
						// Check if the message is ready (fully received)
						auto firstPacket = itPacket;
						std::vector<uint8_t> msg;
						auto expectedPacketId = itPacket->id();
						auto msgLastPacket = [&]()
						{
							++itPacket;
							while (itPacket != itEnd && itPacket->id() == expectedPacketId)
							{
								if (itPacket->type() == Packet::Type::LastFragment)
								{
									// Last fragment reached, the message is full
									msg.insert(msg.cend(), itPacket->data(), itPacket->data() + itPacket->size());
									return itPacket;
								}
								else if (itPacket->type() != Packet::Type::Fragment)
								{
									// If we reach this, we likely recieved a malformed packet / hack attempt
									msg.clear();
									return itPacket;
								}

								msg.insert(msg.cend(), itPacket->data(), itPacket->data() + itPacket->size());
								++itPacket;
								++expectedPacketId;
							}
							msg.clear();
							return itPacket;
						}();
						if (!msg.empty())
						{
							// We do have a message
							messagesReady.push_back(std::move(msg));
							newestProcessedPacket = msgLastPacket;
							itPacket = msgLastPacket;
							++itPacket;
						}
						else
						{
							// Move the iterator after the last packet found
							itPacket = msgLastPacket;
							if (itPacket != itEnd)
								++itPacket;
						}
					}
					else
					{
						++itPacket;
					}
				}

				// Remove every processed and partial packets until the last one processed included
				if (!messagesReady.empty())
					mPendingQueue.erase(mPendingQueue.cbegin(), std::next(newestProcessedPacket));

				return messagesReady;
			}
		}
	}
}