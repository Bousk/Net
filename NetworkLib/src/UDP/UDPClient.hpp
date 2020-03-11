#pragma once

#include "Address.hpp"
#include "DistantClient.hpp"
#include "Simulator.hpp"
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
		namespace Messages
		{
			class Base;
		}
		namespace UDP
		{
			/*
			Client is the main entry point to send and receive data from a given UDP port.
			
			To use a UDP client, create a Client instance then call init(port) and register channels onto it.
			
			An application loop should do the following
			- frame start -
			> receive
			>> receive pending data from the system
			> poll
			>> retrieve pending messages to handle
			> connect, disconnect, sendTo
			>> deal with your existing clients, or reach new ones
			>> to accept an incoming connection, simply call connect to the requester address
			> process Send
			>> do send data to existing clients
			- frame end -

			receive & processSend CANNOT be called concurrently.

			Once you're done with the client, you should release it. Once released, no more data can be sent
			nor received on this port.
			*/
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

			#if BOUSKNET_ALLOW_NETWORK_SIMULATOR == BOUSKNET_SETTINGS_ENABLED
				Simulator& simulator() { return mSimulator; }
				const Simulator& simulator() const { return mSimulator; }
			#endif // BOUSKNET_ALLOW_NETWORK_SIMULATOR == BOUSKNET_SETTINGS_ENABLED

				// Initialise socket to send and receive data on the given port
				bool init(uint16 port);
				void release();

				static void SetTimeout(std::chrono::milliseconds timeout);
				static std::chrono::milliseconds GetTimeout();

				// Can be called anytime from any thread
				void connect(const Address& addr);
				// Can be called anytime from any thread
				void disconnect(const Address& addr);
				// Can be called anytime from any thread
				void sendTo(const Address& target, std::vector<uint8>&& data, uint32 channelIndex);

				// This performs operations on existing clients. Must not be called while calling receive
				void processSend();
				// This performs operations on existing clients. Must not be called while calling processSend
				void receive();
				// Extract ready messages. Can be called anytime from any thread but each message is unique and polled only once
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

				struct Operation {
				public:
					enum class Type {
						Connect,
						SendTo,
						Disconnect,
					};
				public:
					static Operation Connect(const Address& target) { return Operation(Type::Connect, target); }
					static Operation SendTo(const Address& target, std::vector<uint8>&& data, uint32 channel) { return Operation(Type::SendTo, target, std::move(data), channel); }
					static Operation Disconnect(const Address& target) { return Operation(Type::Disconnect, target); }

					Operation(Type type, const Address& target)
						: mType(type)
						, mTarget(target)
					{}
					Operation(Type type, const Address& target, std::vector<uint8>&& data, uint32 channel)
						: mType(type)
						, mTarget(target)
						, mData(std::move(data))
						, mChannel(channel)
					{}

					Type mType;
					Address mTarget;
					std::vector<uint8> mData;
					uint32 mChannel{ 0 };
				};
				std::mutex mOperationsLock;
				using OperationsLock = std::lock_guard<std::mutex>;
				std::vector<Operation> mPendingOperations;

			#if BOUSKNET_ALLOW_NETWORK_SIMULATOR == BOUSKNET_SETTINGS_ENABLED
				Simulator mSimulator;
			#endif // BOUSKNET_ALLOW_NETWORK_SIMULATOR == BOUSKNET_SETTINGS_ENABLED
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
