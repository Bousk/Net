#pragma once

#include "Sockets.hpp"

#include <vector>
#include <memory>

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
			class Client
			{
				friend class DistantClient;
			public:
				Client();
				Client(const Client&) = delete;
				Client(Client&&) = delete;
				Client& operator=(const Client&) = delete;
				Client& operator=(Client&&) = delete;
				~Client();

				bool init(uint16_t port);
				void release();
				void sendTo(const sockaddr_storage& target, std::vector<uint8_t>&& data);
				void receive();
				std::vector<std::unique_ptr<Messages::Base>>&& poll();

			private:
				DistantClient& getClient(const sockaddr_storage& clientAddr);

			private:
				void onMessageReady(std::unique_ptr<Messages::Base>&& msg);

			private:
				SOCKET mSocket{ INVALID_SOCKET };
				std::vector<std::unique_ptr<DistantClient>> mClients;
				std::vector<std::unique_ptr<Messages::Base>> mMessages;
			};
		}
	}
}