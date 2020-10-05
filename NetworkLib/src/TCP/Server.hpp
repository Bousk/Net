#pragma once

#include <Sockets.hpp>
#include <Types.hpp>
#include <TCP/Client.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace Bousk
{
	namespace Network
	{
		namespace Messages {
			class Base;
		}
		namespace TCP
		{
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
				void update();
				void stop();

				// Cannot be called during update
				void accept(uint64 clientid);
				// Cannot be called during update
				void refuse(uint64 clientid);

				// Cannot be called during update
				bool sendTo(uint64 clientid, const uint8* data, size_t len);
				// Cannot be called during update
				bool sendToAll(const uint8* data, size_t len);
				std::vector<std::unique_ptr<Messages::Base>> poll();

			private:
				std::map<uint64, Client> mClients;
				std::mutex mMessagesLock;
				using MessagesLock = std::lock_guard<decltype(mMessagesLock)>;
				std::vector<std::unique_ptr<Messages::Base>> mMessages;
				SOCKET mSocket{ INVALID_SOCKET };
			};
		}
	}
}