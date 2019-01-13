#include "UDP/UDPClient.hpp"
#include "UDP/DistantClient.hpp"
#include "Messages.hpp"
#include "Errors.hpp"

#include <algorithm>
#include <cstring>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			Client::Client()
			{}
			Client::~Client()
			{}

			bool Client::init(uint16_t port)
			{
				release();
				mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
				if (mSocket == INVALID_SOCKET)
					return false;

				sockaddr_in addr;
				addr.sin_addr.s_addr = INADDR_ANY;
				addr.sin_port = htons(port);
				addr.sin_family = AF_INET;
				int res = bind(mSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
				if (res != 0)
					return false;

				if (!SetNonBlocking(mSocket))
					return false;

				return true;
			}
			void Client::release()
			{
				if (mSocket != INVALID_SOCKET)
					CloseSocket(mSocket);
				mSocket = INVALID_SOCKET;
			}
			void Client::sendTo(const sockaddr_storage& target, std::vector<uint8_t>&& data)
			{
				auto& client = getClient(target);
				client.send(std::move(data));
			}
			void Client::processSend()
			{
				for (auto& client : mClients)
					client->processSend();
			}
			void Client::receive()
			{
				for (;;)
				{
					Datagram datagram;
					sockaddr_in from{ 0 };
					socklen_t fromlen = sizeof(from);
					int ret = recvfrom(mSocket, reinterpret_cast<char*>(&datagram), Datagram::BufferMaxSize, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
					if (ret > 0)
					{
						if (ret > Datagram::HeaderSize)
						{
							datagram.datasize = ret - Datagram::HeaderSize;
							auto& client = getClient(reinterpret_cast<sockaddr_storage&>(from));
							client.onDatagramReceived(std::move(datagram));
						}
						else
						{
							//!< Something is wrong, unexpected datagram
						}
					}
					else
					{
						if (ret < 0)
						{
							//!< Error handling
							const auto err = Errors::Get();
							if (err != Errors::WOULDBLOCK)
							{
								//!< Log that error
							}
						}
						return;
					}
				}
			}
			std::vector<std::unique_ptr<Messages::Base>> Client::poll()
			{
				return std::move(mMessages);
			}

			DistantClient& Client::getClient(const sockaddr_storage& clientAddr)
			{
				auto itClient = std::find_if(mClients.begin(), mClients.end(), [&](const std::unique_ptr<DistantClient>& client) { return memcmp(&(client->address()), &clientAddr, sizeof(sockaddr_storage)); });
				if (itClient != mClients.end())
					return *(itClient->get());

				mClients.emplace_back(std::make_unique<DistantClient>(*this, clientAddr));
				return *(mClients.back());
			}
			void Client::onMessageReady(std::unique_ptr<Messages::Base>&& msg)
			{
				mMessages.push_back(std::move(msg));
			}
		}
	}
}
