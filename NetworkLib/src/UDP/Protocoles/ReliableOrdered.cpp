#include "UDP/Protocoles/ReliableOrdered.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cassert>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			namespace Protocols
			{
				namespace ReliableOrdered
				{
					void Multiplexer::queue(std::vector<uint8_t>&& msgData)
					{
						assert(msgData.size() <= Packet::MaxMessageSize);
						if (msgData.size() > Packet::DataMaxSize)
						{
							size_t queuedSize = 0;
							while (queuedSize < msgData.size())
							{
								const auto fragmentSize = std::min(Packet::DataMaxSize, static_cast<uint16_t>(msgData.size() - queuedSize));
								mQueue.resize(mQueue.size() + 1);
								Packet& packet = mQueue.back().packet();
								packet.header.id = mNextId++;
								packet.header.type = ((queuedSize == 0) ? Packet::Type::FirstFragment : Packet::Type::Fragment);
								packet.header.size = fragmentSize;
								memcpy(packet.data(), msgData.data() + queuedSize, fragmentSize);
								queuedSize += fragmentSize;
							}
							mQueue.back().packet().header.type = Packet::Type::LastFragment;
							assert(queuedSize == msgData.size());
						}
						else
						{
							//!< Single packet
							mQueue.resize(mQueue.size() + 1);
							Packet& packet = mQueue.back().packet();
							packet.header.id = mNextId++;
							packet.header.type = Packet::Type::Packet;
							packet.header.size = static_cast<uint16_t>(msgData.size());
							memcpy(packet.data(), msgData.data(), msgData.size());
						}
					}
					size_t Multiplexer::serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID datagramId)
					{
						size_t serializedSize = 0;
						for (auto& packetHolder : mQueue)
						{
							if (!packetHolder.shouldSend())
								continue;
							const auto& packet = packetHolder.packet();
							if (serializedSize + packet.size() > buffersize)
								break; //!< Not enough room for next packet

							memcpy(buffer, packet.buffer(), packet.size());
							serializedSize += packet.size();
							buffer += packet.size();

							//!< Once the packet has been serialized into a datagram, save which datagram it's been included into
							packetHolder.onSent(datagramId);
						}
						return serializedSize;
					}

					void Multiplexer::onDatagramAcked(Datagram::ID datagramId)
					{
						mQueue.erase(std::remove_if(mQueue.begin(), mQueue.end()
							, [&](const ReliablePacket& packetHolder) { return packetHolder.isIncludedIn(datagramId); })
						, mQueue.cend());
					}
					void Multiplexer::onDatagramLost(Datagram::ID datagramId)
					{
						for (auto& packetHolder : mQueue)
						{
							if (packetHolder.isIncludedIn(datagramId))
								packetHolder.resend();
						}
					}

					void Demultiplexer::onDataReceived(const uint8_t* data, const size_t datasize)
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
					void Demultiplexer::onPacketReceived(const Packet* pckt)
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
					std::vector<std::vector<uint8_t>> Demultiplexer::process()
					{
						std::vector<std::vector<uint8_t>> messagesReady;

						auto itPacket = mPendingQueue.cbegin();
						auto itEnd = mPendingQueue.cend();
						std::vector<Packet>::const_iterator newestProcessedPacket;
						Packet::Id expectedPacketId = mLastProcessed + 1;
						//!< Our queue is ordered, just go through and reassemble packets
						while (itPacket != itEnd && itPacket->id() == expectedPacketId)
						{
							if (itPacket->type() == Packet::Type::Packet)
							{
								//!< Full packet, just take it
								std::vector<uint8_t> msg(itPacket->data(), itPacket->data() + itPacket->datasize());
								messagesReady.push_back(std::move(msg));
								newestProcessedPacket = itPacket;
								++itPacket;
								++expectedPacketId;
							}
							else if (itPacket->type() == Packet::Type::FirstFragment)
							{
								//!< Check if the message is ready (fully received)
								std::vector<uint8_t> msg = [&]()
								{
									std::vector<uint8_t> msg(itPacket->data(), itPacket->data() + itPacket->datasize());
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
}