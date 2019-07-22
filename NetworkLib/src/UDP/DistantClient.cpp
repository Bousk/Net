#include "UDP/DistantClient.hpp"
#include "UDP/UDPClient.hpp"
#include "UDP/Datagram.hpp"
#include "Messages.hpp"

#include <cassert>
#include <cstring>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			DistantClient::DistantClient(Client& client, const sockaddr_storage& addr)
				: mClient(client)
			{
				memcpy(&mAddress, &addr, sizeof(addr));
			}
			void DistantClient::send(std::vector<uint8_t>&& data, uint32_t canalIndex)
			{
				mChannelsHandler.queue(std::move(data), canalIndex);
			}
			bool DistantClient::fillDatagram(Datagram& dgram)
			{
				dgram.header.ack = htons(mReceivedAcks.lastAck());
				dgram.header.previousAcks = mReceivedAcks.previousAcksMask();

				dgram.datasize = mChannelsHandler.serialize(dgram.data.data(), Datagram::DataMaxSize, mNextDatagramIdToSend);
				if (dgram.datasize > 0)
				{
					dgram.header.id = htons(mNextDatagramIdToSend);
					++mNextDatagramIdToSend;
					return true;
				}
				return false;
			}
			void DistantClient::processSend()
			{
				for (;;)
				{
					Datagram datagram;
					if (!fillDatagram(datagram))
						break;

					sendto(mClient.mSocket, reinterpret_cast<const char*>(&datagram), static_cast<int>(datagram.size()), 0, reinterpret_cast<const sockaddr*>(&mAddress), sizeof(mAddress));
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
				//!< Dispatch data
				onDataReceived(datagram.data.data(), datagram.datasize);
			}

			void DistantClient::onDatagramSentAcked(Datagram::ID datagramId)
			{
				mChannelsHandler.onDatagramAcked(datagramId);
			}
			void DistantClient::onDatagramSentLost(Datagram::ID datagramId)
			{
				mChannelsHandler.onDatagramLost(datagramId);
			}
			void DistantClient::onDatagramReceivedLost(Datagram::ID datagramId)
			{}
			void DistantClient::onDataReceived(const uint8_t* data, const size_t datasize)
			{
				mChannelsHandler.onDataReceived(data, datasize);
				auto receivedMessages = mChannelsHandler.process();
				for (auto&& msg : receivedMessages)
				{
					onMessageReady(std::make_unique<Messages::UserData>(std::move(msg)));
				}
			}
			void DistantClient::onMessageReady(std::unique_ptr<Messages::Base>&& msg)
			{
				memcpy(&(msg->from), &mAddress, sizeof(msg->from));
				mClient.onMessageReady(std::move(msg));
			}
		}
	}
}
