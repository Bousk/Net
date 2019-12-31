#include "Server.hpp"

#include "Sockets.hpp"
#include "TCP/Client.hpp"
#include "Messages.hpp"

#include <map>
#include <list>
#include <cassert>
#include <cstring>

namespace Bousk
{
	namespace Network
	{
		namespace TCP
		{
			class ServerImpl
			{
			public:
				ServerImpl() = default;
				~ServerImpl();

				bool start(unsigned short _port);
				void stop();
				void update();
				bool sendTo(uint64_t clientid, const unsigned char* data, unsigned int len);
				bool sendToAll(const unsigned char* data, unsigned int len);
				std::unique_ptr<Messages::Base> poll();

			private:
				std::map<uint64_t, Client> mClients;
				std::list<std::unique_ptr<Messages::Base>> mMessages;
				SOCKET mSocket{ INVALID_SOCKET };
			};
			ServerImpl::~ServerImpl()
			{
				stop();
			}

			bool ServerImpl::start(unsigned short _port)
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
			void ServerImpl::stop()
			{
				for (auto& client : mClients)
					client.second.disconnect();
				mClients.clear();
				if (mSocket != INVALID_SOCKET)
					CloseSocket(mSocket);
				mSocket = INVALID_SOCKET;
			}
			void ServerImpl::update()
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
			bool ServerImpl::sendTo(uint64_t clientid, const unsigned char* data, unsigned int len)
			{
				auto itClient = mClients.find(clientid);
				return itClient != mClients.end() && itClient->second.send(data, len);
			}
			bool ServerImpl::sendToAll(const unsigned char* data, unsigned int len)
			{
				bool ret = true;
				for (auto& client : mClients)
					ret &= client.second.send(data, len);
				return ret;
			}
			std::unique_ptr<Messages::Base> ServerImpl::poll()
			{
				if (mMessages.empty())
					return nullptr;

				auto msg = std::move(mMessages.front());
				mMessages.pop_front();
				return msg;
			}
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			Server::Server() {}
			Server::~Server() {}
			Server::Server(Server&& other)
				: mImpl(std::move(other.mImpl))
			{}
			Server& Server::operator=(Server&& other)
			{
				mImpl = std::move(other.mImpl);
				return *this;
			}
			bool Server::start(unsigned short _port)
			{
				if (!mImpl)
					mImpl = std::make_unique<ServerImpl>();
				return mImpl && mImpl->start(_port);
			}
			void Server::stop() { if (mImpl) mImpl->stop(); }
			void Server::update() { if (mImpl) mImpl->update(); }
			bool Server::sendTo(uint64_t clientid, const unsigned char* data, unsigned int len) { return mImpl && mImpl->sendTo(clientid, data, len); }
			bool Server::sendToAll(const unsigned char* data, unsigned int len) { return mImpl && mImpl->sendToAll(data, len); }
			std::unique_ptr<Messages::Base> Server::poll() { return mImpl ? mImpl->poll() : nullptr; }
		}
	}
}
