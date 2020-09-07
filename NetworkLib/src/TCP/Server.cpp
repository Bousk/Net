#include <TCP/Server.hpp>

#include <Messages.hpp>

#include <cassert>

namespace Bousk
{
	namespace Network
	{
		namespace TCP
		{
			Server::Server()
			{}
			Server::~Server()
			{
				stop();
			}

			bool Server::start(unsigned short _port)
			{
				assert(mSocket == INVALID_SOCKET);
				if (mSocket != INVALID_SOCKET)
					stop();
				mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (mSocket == INVALID_SOCKET)
					return false;

				if (!SetReuseAddr(mSocket) || !SetNonBlocking(mSocket))
				{
					stop();
					return false;
				}

				Address addr = Address::Any(Address::Type::IPv4, _port);
				if (!addr.bind(mSocket))
				{
					stop();
					return false;
				}
				if (listen(mSocket, SOMAXCONN) != 0)
				{
					stop();
					return false;
				}
				return true;
			}
			void Server::stop()
			{
				for (auto& client : mClients)
					client.second.disconnect();
				mClients.clear();
				if (mSocket != INVALID_SOCKET)
					CloseSocket(mSocket);
				mSocket = INVALID_SOCKET;
			}
			void Server::update()
			{
				if (mSocket == INVALID_SOCKET)
					return;

				//!< accept up to 10 new clients each time
				for (int accepted = 0; accepted < 10; ++accepted)
				{
					Address addr;
					SOCKET newClientSocket;
					if (!addr.accept(mSocket, newClientSocket))
						break;
					Client newClient;
					if (newClient.init(std::move(newClientSocket), addr))
					{
						auto message = std::make_unique<Messages::Connection>(newClient.address(), newClient.id(), Messages::Connection::Result::Success);
						mMessages.push_back(std::move(message));
						mClients[newClient.id()] = std::move(newClient);
					}
				}

				//!< update connected clients
				//!< receives up to 1 message per client
				//!< remove disconnected clients
				for (auto itClient = mClients.begin(); itClient != mClients.end(); )
				{
					auto& client = itClient->second;
					auto msg = client.poll();
					if (msg)
					{
						if (msg->is<Messages::Disconnection>())
						{
							itClient = mClients.erase(itClient);
						}
						else
						{
							++itClient;
						}
						mMessages.push_back(std::move(msg));
					}
					else
						++itClient;
				}
			}
			void Server::accept(uint64 clientid)
			{
				;
			}
			void Server::refuse(uint64 clientid)
			{
				;
			}
			bool Server::sendTo(uint64 clientid, const uint8* data, size_t len)
			{
				auto itClient = mClients.find(clientid);
				return itClient != mClients.end() && itClient->second.send(data, len);
			}
			bool Server::sendToAll(const uint8* data, size_t len)
			{
				bool ret = true;
				for (auto& client : mClients)
					ret &= client.second.send(data, len);
				return ret;
			}
			std::unique_ptr<Messages::Base> Server::poll()
			{
				if (mMessages.empty())
					return nullptr;

				auto msg = std::move(mMessages.front());
				mMessages.pop_front();
				return msg;
			}
		}
	}
}
