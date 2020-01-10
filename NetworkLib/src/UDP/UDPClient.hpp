#pragma once

#include "Sockets.hpp"
#include "Types.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace Bousk
{
	namespace Network
	{
		class Address;
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

				static void SetTimeout(std::chrono::milliseconds timeout);
				static std::chrono::milliseconds GetTimeout();

				bool connect(const Address& addr);
				void disconnect(const Address& addr);
				
				void sendTo(const sockaddr_storage& target, std::vector<uint8_t>&& data, uint32_t canalIndex);
				void processSend();
				
				void receive();
				std::vector<std::unique_ptr<Messages::Base>> poll();

			private:
				DistantClient* getClient(const Address& clientAddr, bool create = false);
				void setupChannels(DistantClient& client);

			private:
				void onMessageReady(std::unique_ptr<Messages::Base>&& msg);

			private:
				SOCKET mSocket{ INVALID_SOCKET };
				std::vector<std::unique_ptr<DistantClient>> mClients;
				uint64 mClientIdsGenerator{ 0 };
				std::mutex mMessagesLock;
				using MessagesLock = std::lock_guard<std::mutex>;
				std::vector<std::unique_ptr<Messages::Base>> mMessages;

				std::vector<std::function<void(DistantClient&)>> mRegisteredChannels;
			};

			template<class T>
			void Client::registerChannel()
			{
				assert(mSocket == INVALID_SOCKET); // Don't add channels after being initialized !!!
				mRegisteredChannels.push_back([](DistantClient& distantClient) { distantClient.registerChannel<T>(); });
			}
		}
	}
}
