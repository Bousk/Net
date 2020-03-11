#pragma once

#include <Types.hpp>
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
				Server(Server&&);
				Server& operator=(Server&&);
				~Server();

				bool start(unsigned short _port);
				void stop();
				void update();
				bool sendTo(uint64 clientid, const uint8* data, size_t len);
				bool sendToAll(const uint8* data, size_t len);
				std::unique_ptr<Messages::Base> poll();

			private:
				std::unique_ptr<ServerImpl> mImpl;
			};
		}
	}
}