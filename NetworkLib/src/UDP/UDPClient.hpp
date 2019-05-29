#pragma once

#include "Sockets.hpp"

#include <vector>
#include <memory>
#include <functional>

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
            class DistantClient;
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

				template<class T>
				void registerChannel();

				bool init(uint16_t port);
				void release();
				void sendTo(const sockaddr_storage& target, std::vector<uint8_t>&& data, uint32_t canalIndex);
				void processSend();
				void receive();
				std::vector<std::unique_ptr<Messages::Base>> poll();

			private:
				DistantClient& getClient(const sockaddr_storage& clientAddr);
				void setupChannels(DistantClient& client);

			private:
				void onMessageReady(std::unique_ptr<Messages::Base>&& msg);

			private:
				SOCKET mSocket{ INVALID_SOCKET };
				std::vector<std::unique_ptr<DistantClient>> mClients;
				std::vector<std::unique_ptr<Messages::Base>> mMessages;

				std::vector<std::function<void(DistantClient&)>> mRegisteredChannels;
			};

			template<class T>
			void Client::registerChannel()
			{
				mRegisteredChannels.push_back([](DistantClient& distantClient) { distantClient.registerChannel<T>(); });
			}
		}
	}
}
