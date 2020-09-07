#pragma once

#include <Sockets.hpp>
#include <Types.hpp>
#include <TCP/Client.hpp>

#include <list>
#include <map>
#include <memory>

namespace Bousk
{
	namespace Network
	{
		namespace Messages {
			class Base;
		}
		namespace TCP
		{
			class ServerImpl;
			class Server
			{
			public:
				Server();
				Server(const Server&) = delete;
				Server& operator=(const Server&) = delete;
				Server(Server&&) = delete;;
				Server& operator=(Server&&) = delete;;
				~Server();

				bool start(unsigned short _port);
				void stop();
				void update();
				void accept(uint64 clientid);
				void refuse(uint64 clientid);
				bool sendTo(uint64 clientid, const uint8* data, size_t len);
				bool sendToAll(const uint8* data, size_t len);
				std::unique_ptr<Messages::Base> poll();

			private:
				std::map<uint64, Client> mClients;
				std::list<std::unique_ptr<Messages::Base>> mMessages;
				SOCKET mSocket{ INVALID_SOCKET };
			};
		}
	}
}