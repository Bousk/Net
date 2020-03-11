#include "UDP/DistantClient.hpp"
#include "UDP/UDPClient.hpp"
#include "UDP/Datagram.hpp"
#include "Serialization/Serializer.hpp"
#include "Serialization/Deserializer.hpp"
#include "Messages.hpp"
#include "Settings.hpp"
#include "Utils.hpp"

#include <cassert>
#include <cstring>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			std::chrono::milliseconds DistantClient::sTimeout = BOUSKNET_DEFAULT_UDP_TIMEOUT;

			DistantClient::DistantClient(Client& client, const Address& addr, uint64 clientid)
				: mClient(client)
				, mAddress(addr)
				, mClientId(clientid)
				, mConnectionStartTime(Utils::Now())
				, mLastKeepAlive(Utils::Now())
			{}
			void DistantClient::onConnectionSent()
			{
				if (mState == State::None)
				{
					mState = State::ConnectionSent;
					maintainConnection();
				}
				else if (mState == State::ConnectionReceived)
				{
					// We sent a connection and had received one : we do connect !
					onConnected();
				}
			}
			void DistantClient::onConnectionReceived()
			{
				if (mState == State::None)
				{
					mState = State::ConnectionReceived;
					maintainConnection();
					// Push incoming connection request to client
					mClient.onMessageReady(std::make_unique<Messages::IncomingConnection>(mAddress, mClientId));
				}
				else if (mState == State::ConnectionSent)
				{
					// We received a connection and had sent one : we do connect !
					onConnected();
				}
			}
			void DistantClient::maintainConnection()
			{
				mLastKeepAlive = Utils::Now();
			}
			void DistantClient::onConnected()
			{
				mState = State::Connected;
				maintainConnection();
				onMessageReady(std::make_unique<Messages::Connection>(mAddress, mClientId, Messages::Connection::Result::Success));
				// Dispatch pending messages now
				for (auto&& pendingMessage : mPendingMessages)
				{
					onMessageReady(std::move(pendingMessage));
				}
			}
			void DistantClient::onDisconnectionFromOtherEnd()
			{
				if (isConnecting())
				{
					onConnectionRefused();
				}
				else if (isConnected())
				{
					mState = State::Disconnecting;
					mDisconnectionReason = DisconnectionReason::Disconnected;
				}
			}
			void DistantClient::onConnectionLost()
			{
				if (isConnected())
				{
					// Start disconnecting and save the reason to notify later
					mState = State::Disconnecting;
					mDisconnectionReason = DisconnectionReason::Lost;
				}
			}
			void DistantClient::onConnectionRefused()
			{
				if (mState == State::ConnectionSent)
				{
					mDisconnectionReason = DisconnectionReason::Refused;
				}
				mState = State::Disconnecting;
			}
			void DistantClient::onConnectionTimedOut()
			{
				if (mState == State::ConnectionSent)
				{
					mDisconnectionReason = DisconnectionReason::ConnectionTimedOut;
				}
				mState = State::Disconnecting;
			}
			void DistantClient::connect()
			{
				onConnectionSent();
			}
			void DistantClient::disconnect()
			{
				// To disconnect, stop sending packets
				// We switch to a disconnecting state to prevent a reconnection from happening
				// when receiving packets from the other end right after disconnecting locally
				mState = State::Disconnecting;
			}
			void DistantClient::send(std::vector<uint8>&& data, uint32 channelIndex)
			{
				// If we're sending data, we're requesting a connection
				onConnectionSent();
				mChannelsHandler.queue(std::move(data), channelIndex);
			}
			void DistantClient::fillDatagramHeader(Datagram& dgram, Datagram::Type type)
			{
				dgram.header.ack = htons(mReceivedAcks.lastAck());
				dgram.header.previousAcks = mReceivedAcks.previousAcksMask();
				dgram.header.id = htons(mNextDatagramIdToSend);
				dgram.header.type = type;
				++mNextDatagramIdToSend;
			}
			void DistantClient::send(const Datagram& dgram)
			{
				int ret = mAddress.sendTo(mClient.mSocket, reinterpret_cast<const char*>(&dgram), dgram.size());
				if (ret < 0)
				{
					// Error
				}
			}
			void DistantClient::processSend(const size_t maxDatagrams /*= 0*/)
			{
				const auto now = Utils::Now();
				// We do send data during connection process in order to keep it available before we accept it
				if (isConnecting() || isConnected())
				{
					for (size_t loop = 0; maxDatagrams == 0 || loop < maxDatagrams; ++loop)
					{
						Datagram datagram;
						datagram.datasize = mChannelsHandler.serialize(datagram.data.data(), Datagram::DataMaxSize, mNextDatagramIdToSend);
						if (datagram.datasize > 0)
						{
							fillDatagramHeader(datagram, Datagram::Type::ConnectedData);
							send(datagram);
						}
						else
						{
							if (loop == 0)
							{
								// Nothing to send this time, so send a keep alive to maintain connection
								fillKeepAlive(datagram);
								send(datagram);
							}
							break;
						}
					}
				}
				if (isDisconnecting())
				{
					if (now > mLastKeepAlive + 2 * GetTimeout())
					{
						// After 2 timeouts we mark it disconnected
						// This leaves enough time to each end to notice and disconnects its distant client
						mState = State::Disconnected;
						// Do notify the disconnection, if needed
						// We notify it as latest as possible so when user received the Disconnection message, he can send a new connection request right away
						switch (mDisconnectionReason)
						{
							case DisconnectionReason::Disconnected:
								mClient.onMessageReady(std::make_unique<Messages::Disconnection>(mAddress, mClientId, Messages::Disconnection::Reason::Disconnected));
								break;
							case DisconnectionReason::Lost:
								mClient.onMessageReady(std::make_unique<Messages::Disconnection>(mAddress, mClientId, Messages::Disconnection::Reason::Lost));
								break;
							case DisconnectionReason::Refused:
								mClient.onMessageReady(std::make_unique<Messages::Connection>(mAddress, mClientId, Messages::Connection::Result::Refused));
								break;
							case DisconnectionReason::ConnectionTimedOut:
								mClient.onMessageReady(std::make_unique<Messages::Connection>(mAddress, mClientId, Messages::Connection::Result::TimedOut));
								break;
						}
					}
					else if (mDisconnectionReason != DisconnectionReason::None && mDisconnectionReason != DisconnectionReason::Lost)
					{
						// Send disconnection datagrams while disconnecting to inform the other end that's a normal termination
						Datagram datagram;
						fillDatagramHeader(datagram, Datagram::Type::Disconnection);
						send(datagram);
					}
				}
				else if (isConnected() && now > mLastKeepAlive + GetTimeout())
				{
					onConnectionLost();
				}
				else if (isConnecting() && now > mConnectionStartTime + GetTimeout())
				{
					// Connection hasn't been accepted within timeframe
					onConnectionTimedOut();
				}
			}
			void DistantClient::fillKeepAlive(Datagram& dgram)
			{
				fillDatagramHeader(dgram, Datagram::Type::KeepAlive);
				// Do notify the other end if we're supposed to be connected or requesting the connection
				Serialization::Serializer serializer;
				serializer.write(mState == State::ConnectionSent || isConnected());
				memcpy(dgram.data.data(), serializer.buffer(), serializer.bufferSize());
				dgram.datasize = static_cast<uint16>(serializer.bufferSize());
			}
			void DistantClient::handleKeepAlive(const uint8* data, const uint16 datasize)
			{
				maintainConnection();
				if (mState == State::None || isConnecting())
				{
					Serialization::Deserializer deserializer(data, datasize);
					bool isConnectedKeepAlive = false;
					if (deserializer.read(isConnectedKeepAlive) && isConnectedKeepAlive)
					{
						onConnectionReceived();
					}
				}
			}
			void DistantClient::onDatagramReceived(Datagram&& datagram)
			{
				const auto datagramid = ntohs(datagram.header.id);
				//!< Update the received acks tracking
				mReceivedAcks.update(datagramid, 0, true);
				//!< Update the send acks tracking
				mSentAcks.update(ntohs(datagram.header.ack), datagram.header.previousAcks, true);
				//!< Ignore duplicate
				if (!mReceivedAcks.isNewlyAcked(datagramid))
				{
					return;
				}

				//!< Handle loss on reception
				const auto lostDatagrams = mReceivedAcks.loss();
				for (const auto receivedLostDatagram : lostDatagrams)
				{
					onDatagramReceivedLost(receivedLostDatagram);
				}
				//!< Handle loss on send
				const auto datagramsSentLost = mSentAcks.loss();
				for (const auto sendLoss : datagramsSentLost)
				{
					onDatagramSentLost(sendLoss);
				}
				//!< Mark new send acked
				const auto datagramsSentAcked = mSentAcks.getNewAcks();
				for (const auto sendAcked : datagramsSentAcked)
				{
					onDatagramSentAcked(sendAcked);
				}
				switch (datagram.header.type)
				{
				case Datagram::Type::ConnectedData:
				{
					//!< Dispatch data
					onDataReceived(datagram.data.data(), datagram.datasize);
				} break;
				case Datagram::Type::KeepAlive:
				{
					handleKeepAlive(datagram.data.data(), datagram.datasize);
				} break;
				case Datagram::Type::Disconnection:
				{
					onDisconnectionFromOtherEnd();
				}
				}
			}

			void DistantClient::onDatagramSentAcked(Datagram::ID datagramId)
			{
				mChannelsHandler.onDatagramAcked(datagramId);
			}
			void DistantClient::onDatagramSentLost(Datagram::ID datagramId)
			{
				mChannelsHandler.onDatagramLost(datagramId);
			}
			void DistantClient::onDatagramReceivedLost(Datagram::ID)
			{}
			void DistantClient::onDataReceived(const uint8* data, const uint16 datasize)
			{
				// If we receive data, the other end is requesting a connection
				onConnectionReceived();
				mChannelsHandler.onDataReceived(data, datasize);
				auto receivedMessages = mChannelsHandler.process(isConnected());
				for (auto&& msg : receivedMessages)
				{
					onMessageReady(std::make_unique<Messages::UserData>(mAddress, mClientId, std::move(msg)));
				}
			}
			void DistantClient::onMessageReady(std::unique_ptr<Messages::Base>&& msg)
			{
				if (isConnected())
				{
					mClient.onMessageReady(std::move(msg));
				}
				else if (isConnecting())
				{
					mPendingMessages.push_back(std::move(msg));
				}
			}
		}
	}
}
