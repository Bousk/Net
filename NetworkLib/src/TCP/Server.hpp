#pragma once

#include <memory>

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
				Server(Server&&) = default;
				Server& operator=(Server&&) = default;
				~Server();

				bool start(unsigned short _port);
				void stop();
				void update();
				std::unique_ptr<Messages::Base> poll();

			private:
				std::unique_ptr<ServerImpl> mImpl;
		};
	}
}