#include "UDP/Protocols/ReliableOrdered.hpp"
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
				void ReliableOrdered::Multiplexer::queue(std::vector<uint8_t>&& msgData)
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
				size_t ReliableOrdered::Multiplexer::serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID datagramId)
				{
					size_t serializedSize = 0;
					for (auto& packetHolder : mQueue)
					{
						if (!(Utils::SequenceDiff(packetHolder.packet().id(), mFirstAllowedPacket) < Demultiplexer::QueueSize))
							break;
						if (!packetHolder.shouldSend())
							continue;
						const auto& packet = packetHolder.packet();
						if (serializedSize + packet.size() > buffersize)
							continue; //!< Not enough room, let's skip this one for now

						memcpy(buffer, packet.buffer(), packet.size());
						serializedSize += packet.size();
						buffer += packet.size();

						//!< Once the packet has been serialized into a datagram, save which datagram it's been included into
						packetHolder.onSent(datagramId);
					}
					return serializedSize;
				}

				void ReliableOrdered::Multiplexer::onDatagramAcked(Datagram::ID datagramId)
				{
					if (mQueue.empty())
						return;

					mQueue.erase(std::remove_if(mQueue.begin(), mQueue.end()
						, [&](const ReliablePacket& packetHolder) { return packetHolder.isIncludedIn(datagramId); })
						, mQueue.cend());
					if (mQueue.empty())
						mFirstAllowedPacket = mNextId;
					else if (Utils::IsSequenceNewer(mQueue.front().packet().id(), mFirstAllowedPacket))
						mFirstAllowedPacket = mQueue.front().packet().id();
				}
				void ReliableOrdered::Multiplexer::onDatagramLost(Datagram::ID datagramId)
				{
					for (auto& packetHolder : mQueue)
					{
						if (packetHolder.isIncludedIn(datagramId))
							packetHolder.resend();
					}
				}

				void ReliableOrdered::Demultiplexer::onDataReceived(const uint8_t* data, const size_t datasize)
				{
					//!< Extract packets from buffer
					size_t processedDataSize = 0;
					while (processedDataSize < datasize)
					{
						const Packet* pckt = reinterpret_cast<const Packet*>(data);
						if (processedDataSize + pckt->size() > datasize || pckt->datasize() > Packet::DataMaxSize)
						{
							//!< Malformed packet or buffer
							break;
						}
						onPacketReceived(pckt);
						processedDataSize += pckt->size();
						data += pckt->size();
					}
				}
				void ReliableOrdered::Demultiplexer::onPacketReceived(const Packet* pckt)
				{
					if (!Utils::IsSequenceNewer(pckt->id(), mLastProcessed))
						return; //!< Packet is too old

								//!< Find the place for this packet
					const size_t index = pckt->id() % mPendingQueue.size();
					Packet& pendingPacket = mPendingQueue[index];
					if (pendingPacket.datasize() == 0)
					{
						// Slot is available, simply copy data from network into the queue
						pendingPacket = *pckt;
					}
					else
					{
						// Slot is NOT available, packet should already be received, otherwise it's an error
						assert(pendingPacket.id() == pckt->id() && pendingPacket.datasize() == pckt->datasize());
					}
				}
				std::vector<std::vector<uint8_t>> ReliableOrdered::Demultiplexer::process()
				{
					auto ResetPacket = [](Packet& pckt) { pckt.header.size = 0; };
					auto IsPacketValid = [](const Packet& pckt) { return pckt.header.size != 0; };
					std::vector<std::vector<uint8_t>> messagesReady;

					Packet::Id expectedPacketId = mLastProcessed + 1;
					const size_t startIndexOffset = expectedPacketId % mPendingQueue.size();
					//!< Our queue is ordered, just go through and reassemble packets
					for (size_t i = 0; i < mPendingQueue.size(); ++i, ++expectedPacketId)
					{
						const size_t packetIndex = (i + startIndexOffset) % mPendingQueue.size();
						Packet& packet = mPendingQueue[packetIndex];
						if (!IsPacketValid(packet))
							break;
						if (packet.type() == Packet::Type::Packet)
						{
							//!< Full packet, just take it
							std::vector<uint8_t> msg(packet.data(), packet.data() + packet.datasize());
							mLastProcessed = packet.id();
							ResetPacket(packet);
							messagesReady.push_back(std::move(msg));
						}
						else if (packet.type() == Packet::Type::FirstFragment)
						{
							//!< Check if the message is ready (fully received)
							const bool isMessageFull = [=]() mutable
							{
								// Skip first fragment
								++i;
								++expectedPacketId;
								// Iterate through remaining packets to check message is complete
								for (size_t j = i; j < mPendingQueue.size(); ++j, ++expectedPacketId)
								{
									const size_t idx = (j + startIndexOffset) % mPendingQueue.size();
									const Packet& pckt = mPendingQueue[idx];
									if (pckt.id() != expectedPacketId || pckt.datasize() == 0)
										break; // Expected packet not received yet
									if (pckt.type() == Packet::Type::LastFragment)
									{
										//!< Last fragment reached, the message is full
										return true;
									}
									else if (pckt.type() != Packet::Type::Fragment)
									{
										//!< If we reach this, we likely recieved a malformed packet / hack attempt
										break;
									}
								}
								return false;
							}();
							if (!isMessageFull)
								break; // Messages are ordered and next one to extract is not full yet

									   // Start the message with first fragment packet, then skip it
							std::vector<uint8_t> msg(packet.data(), packet.data() + packet.datasize());
							++i;
							++expectedPacketId;
							// Iterate through remaining packets to try to complete it
							for (size_t j = i; j < mPendingQueue.size(); ++i, ++j, ++expectedPacketId)
							{
								const size_t idx = (j + startIndexOffset) % mPendingQueue.size();
								Packet& pckt = mPendingQueue[idx];

								if (pckt.type() == Packet::Type::LastFragment)
								{
									//!< Last fragment reached, the message is full
									msg.insert(msg.cend(), pckt.data(), pckt.data() + pckt.datasize());
									mLastProcessed = pckt.id();
									ResetPacket(pckt);
									messagesReady.push_back(std::move(msg));
									break;
								}
								else if (pckt.type() != Packet::Type::Fragment)
								{
									//!< If we reach this, we likely recieved a malformed packet / hack attempt
									break;
								}

								msg.insert(msg.cend(), pckt.data(), pckt.data() + pckt.datasize());
								ResetPacket(pckt);
							}
						}
						else
						{
							// Messages are ordered and next one to extract is not received yet
							break;
						}
					}
					return messagesReady;
				}
			}
		}
	}
}
