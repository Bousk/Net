#include "UDP/Protocols/UnreliableOrdered.hpp"
#include "Utils.hpp"
#include <cassert>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			namespace Protocols
			{
				void UnreliableOrdered::Multiplexer::queue(std::vector<uint8_t>&& msgData)
				{
					assert(msgData.size() <= Packet::MaxMessageSize);
					if (msgData.size() > Packet::DataMaxSize)
					{
						size_t queuedSize = 0;
						while (queuedSize < msgData.size())
						{
							const auto fragmentSize = std::min(Packet::DataMaxSize, static_cast<uint16_t>(msgData.size() - queuedSize));
							mQueue.resize(mQueue.size() + 1);
							Packet& packet = mQueue.back();
							packet.header.id = mNextId++;
							packet.header.type = ((queuedSize == 0) ? Packet::Type::FirstFragment : Packet::Type::Fragment);
							packet.header.size = fragmentSize;
							memcpy(packet.data(), msgData.data() + queuedSize, fragmentSize);
							queuedSize += fragmentSize;
						}
						mQueue.back().header.type = Packet::Type::LastFragment;
						assert(queuedSize == msgData.size());
					}
					else
					{
						//!< Single packet
						mQueue.resize(mQueue.size() + 1);
						Packet& packet = mQueue.back();
						packet.header.id = mNextId++;
						packet.header.type = Packet::Type::FullMessage;
						packet.header.size = static_cast<uint16_t>(msgData.size());
						memcpy(packet.data(), msgData.data(), msgData.size());
					}
				}
				size_t UnreliableOrdered::Multiplexer::serialize(uint8_t* buffer, const size_t buffersize, const Datagram::ID)
				{
					size_t serializedSize = 0;
					for (auto packetit = mQueue.cbegin(); packetit != mQueue.cend();)
					{
						const auto& packet = *packetit;
						if (serializedSize + packet.size() > buffersize)
							break; //!< Not enough room for next packet

						memcpy(buffer, packet.buffer(), packet.size());
						serializedSize += packet.size();
						buffer += packet.size();

						//!< Once the packet has been serialized into a datagram, remove it from the queue
						packetit = mQueue.erase(packetit);
					}
					return serializedSize;
				}
				void UnreliableOrdered::Demultiplexer::onDataReceived(const uint8_t* data, const size_t datasize)
				{
					//!< Extract packets from buffer
					size_t processedDataSize = 0;
					while (processedDataSize < datasize)
					{
						const Packet* pckt = reinterpret_cast<const Packet*>(data);
						if (processedDataSize + pckt->size() > datasize || pckt->datasize() > Packet::DataMaxSize)
						{
							//!< Malformed packet or buffer
							return;
						}
						onPacketReceived(pckt);
						processedDataSize += pckt->size();
						data += pckt->size();
					}
				}
				void UnreliableOrdered::Demultiplexer::onPacketReceived(const Packet* pckt)
				{
					if (!Utils::IsSequenceNewer(pckt->id(), mLastProcessed))
						return; //!< Packet is too old

								//!< Find the place for this packet, our queue must remain ordered
					if (mPendingQueue.empty() || Utils::IsSequenceNewer(pckt->id(), mPendingQueue.back().id()))
					{
						mPendingQueue.push_back(*pckt);
					}
					else
					{
						//!< Find the first iterator with an id equals to or newer than our packet, we must place the packet before that one
						auto insertLocation = std::find_if(mPendingQueue.cbegin(), mPendingQueue.cend(), [&pckt](const Packet& p) { return p.id() == pckt->id() || Utils::IsSequenceNewer(p.id(), pckt->id()); });
						//!< Make sure we don't insert a packet received multiple times
						if (insertLocation->id() != pckt->id())
						{
							mPendingQueue.insert(insertLocation, *pckt);
						}
					}
				}
				std::vector<std::vector<uint8_t>> UnreliableOrdered::Demultiplexer::process()
				{
					std::vector<std::vector<uint8_t>> messagesReady;

					auto itPacket = mPendingQueue.cbegin();
					auto itEnd = mPendingQueue.cend();
					std::vector<Packet>::const_iterator newestProcessedPacket;
					//!< Our queue is ordered, just go through and reassemble packets
					while (itPacket != itEnd)
					{
						if (itPacket->type() == Packet::Type::FullMessage)
						{
							//!< Full packet, just take it
							std::vector<uint8_t> msg(itPacket->data(), itPacket->data() + itPacket->datasize());
							messagesReady.push_back(std::move(msg));
							newestProcessedPacket = itPacket;
							++itPacket;
						}
						else if (itPacket->type() == Packet::Type::FirstFragment)
						{
							//!< Check if the message is ready (fully received)
							std::vector<uint8_t> msg = [&]()
							{
								std::vector<uint8_t> msg(itPacket->data(), itPacket->data() + itPacket->datasize());
								auto expectedPacketId = itPacket->id();
								++itPacket;
								++expectedPacketId;
								while (itPacket != itEnd && itPacket->id() == expectedPacketId)
								{
									if (itPacket->type() == Packet::Type::LastFragment)
									{
										//!< Last fragment reached, the message is full
										msg.insert(msg.cend(), itPacket->data(), itPacket->data() + itPacket->datasize());
										return msg;
									}
									else if (itPacket->type() != Packet::Type::Fragment)
									{
										//!< If we reach this, we likely recieved a malformed packet / hack attempt
										msg.clear();
										return msg;
									}

									msg.insert(msg.cend(), itPacket->data(), itPacket->data() + itPacket->datasize());
									++itPacket;
									++expectedPacketId;
								}
								msg.clear();
								return msg;
							}();
							if (!msg.empty())
							{
								//!< We do have a message
								messagesReady.push_back(std::move(msg));
								newestProcessedPacket = itPacket;
								//!< Move iterator after the last packet of the message
								++itPacket;
							}
						}
						else
						{
							++itPacket;
						}
					}

					//!< Remove every processed and partial packets until the last one processed included
					if (!messagesReady.empty())
					{
						mLastProcessed = newestProcessedPacket->id();
						mPendingQueue.erase(mPendingQueue.cbegin(), std::next(newestProcessedPacket));
					}

					return messagesReady;
				}
			}
		}
	}
}
