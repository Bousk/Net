#include "UDP/DistantClient.hpp"
#include "UDP/UDPClient.hpp"
#include "UDP/Datagram.hpp"
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
			std::chrono::milliseconds DistantClient::sTimeout = DEFAULT_UDP_TIMEOUT;

			DistantClient::DistantClient(Client& client, const Address& addr, uint64 clientid)
				: mClient(client)
				, mAddress(addr)
				, mClientId(clientid)
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
			void DistantClient::disconnect()
			{
				// To disconnect, stop sending packets
				// We switch to a disconnecting state to prevent a reconnection from happening
				// when receiving packets from the other end right after disconnecting locally
				mState = State::Disconnecting;
			}
			void DistantClient::send(std::vector<uint8>&& data, uint32_t canalIndex)
			{
				// If we're sending data, we're requesting a connection
				onConnectionSent();
				mChannelsHandler.queue(std::move(data), canalIndex);
			}
			void DistantClient::fillDatagramHeader(Datagram& dgram, Datagram::Type type)
			{
				dgram.header.ack = htons(mReceivedAcks.lastAck());
				dgram.header.previousAcks = mReceivedAcks.previousAcksMask();
				dgram.header.id = htons(mNextDatagramIdToSend);
				dgram.header.type = type;
				++mNextDatagramIdToSend;
			}
			void DistantClient::send(const Datagram& dgram) const
			{
				int ret = mAddress.sendTo(mClient.mSocket, reinterpret_cast<const char*>(&dgram), dgram.size());
				if (ret < 0)
				{
					// Error
				}
			}
			void DistantClient::processSend(const size_t maxDatagrams /*= 0*/)
			{
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
								fillDatagramHeader(datagram, Datagram::Type::KeepAlive);
								send(datagram);
							}
							break;
						}
					}
				}
			}
			void DistantClient::onDatagramReceived(Datagram&& datagram)
			{
				assert(datagram.datasize > 0);
				const auto datagramid = ntohs(datagram.header.id);
				//!< Update the received acks tracking
				mReceivedAcks.update(datagramid, 0, true);
				//!< Update the send acks tracking
				mSentAcks.update(ntohs(datagram.header.ack), datagram.header.previousAcks, true);
				//!< Ignore duplicate
				if (!mReceivedAcks.isNewlyAcked(datagramid))
					return;
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
					maintainConnection();
				} break;
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
			void DistantClient::onDataReceived(const uint8_t* data, const size_t datasize)
			{
				// If we receive data, the other end is requesting a connection
				onConnectionReceived();
				mChannelsHandler.onDataReceived(data, datasize);
				auto receivedMessages = mChannelsHandler.process();
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
