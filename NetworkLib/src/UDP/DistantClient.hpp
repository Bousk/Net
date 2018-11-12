#pragma once

#include "UDP/Datagram.hpp"
#include "UDP/AckHandler.hpp"
#include "UDP/PacketHandling.hpp"
#include "Sockets.hpp"

#include <vector>
#include <memory>

class DistantClient_Test;
namespace Bousk
{
	namespace Network
	{
		namespace Messages
		{
			class Base;
		}
		namespace UDP
		{
			class Client;
			struct Datagram;
			class DistantClient
			{
				friend class ::DistantClient_Test;
			public:
				DistantClient(Client& client, const sockaddr_storage& addr);
				DistantClient(const DistantClient&) = delete;
				DistantClient(DistantClient&&) = delete;
				DistantClient& operator=(const DistantClient&) = delete;
				DistantClient& operator=(DistantClient&&) = delete;
				~DistantClient() = default;

				void send(std::vector<uint8_t>&& data);
				void processSend();
				void onDatagramReceived(Datagram&& datagram);

				const sockaddr_storage& address() const { return mAddress; }

			private:
				bool fillDatagram(Datagram& dgram);
				void onDatagramSentAcked(Datagram::ID datagramId);
				void onDatagramSentLost(Datagram::ID datagramId);
				void onDatagramReceivedLost(Datagram::ID datagramId);
				void onDataReceived(const uint8_t* data, const size_t datasize);
				void onMessageReady(std::unique_ptr<Messages::Base>&& msg);

			private:
				Client& mClient;
				Multiplexer mSendQueue;
				Demultiplexer mRecvQueue;
				sockaddr_storage mAddress;
				Datagram::ID mNextDatagramIdToSend{ 0 };
				AckHandler mReceivedAcks;	//!< To detect missing received datagrams and duplicates
				AckHandler mSentAcks;		//!< Which sent datagrams have been acked to detect loss
			};
		}
	}
}
