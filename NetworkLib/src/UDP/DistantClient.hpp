#pragma once

#include <Address.hpp>
#include <Sockets.hpp>
#include <UDP/AckHandler.hpp>
#include <UDP/ChannelsHandler.hpp>
#include <UDP/Datagram.hpp>

#include <chrono>
#include <memory>
#include <vector>

class DistantClient_Test;
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
			class Client;
			struct Datagram;
			class DistantClient
			{
				friend class ::DistantClient_Test;
				enum class State {
					None,
					ConnectionSent,
					ConnectionReceived,
					Connected,
					Disconnecting,
					Disconnected,
				};
				enum class DisconnectionReason {
					None,
					Refused,
					ConnectionTimedOut,
					Disconnected,
					DisconnectedFromOtherEnd,
					Lost,
				};
			public:
				DistantClient(Client& client, const Address& addr, uint64 clientid);
				DistantClient(const DistantClient&) = delete;
				DistantClient(DistantClient&&) = delete;
				DistantClient& operator=(const DistantClient&) = delete;
				DistantClient& operator=(DistantClient&&) = delete;
				~DistantClient() = default;

				static void SetTimeout(std::chrono::milliseconds timeout) { sTimeout = timeout; }
				static std::chrono::milliseconds GetTimeout() { return sTimeout; }

				inline bool isConnecting() const { return mState == State::ConnectionSent || mState == State::ConnectionReceived; }
				inline bool isConnected() const { return mState == State::Connected; }
				inline bool isDisconnecting() const { return mState == State::Disconnecting; }
				inline bool isDisconnected() const { return mState == State::Disconnected; }
			#if BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
				inline bool isInterrupted() const { return mInterrupted; }
				inline bool hasInterruptedClients() const { return mDistantInterrupted; }
			#endif // BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED

				template<class T>
				void registerChannel();

				void connect();
				void disconnect();

				void send(std::vector<uint8>&& data, uint32 channelIndex);
				void processSend(uint8 maxDatagrams = 0);
				void onDatagramReceived(Datagram&& datagram);

				inline const Address& address() const { return mAddress; }
				inline uint64 id() const { return mClientId; }

			private:
				void maintainConnection(bool distantNetworkInterrupted = false);
				void onConnectionSent();
				void onConnectionReceived();
				void onConnected();
				void onDisconnectionFromOtherEnd();
				// Called when this client has its connection interrupted with us.
				void onConnectionInterrupted();
				// Called when this client reports having a connection interruption with one if its clients.
				void onConnectionInterruptedForwarded();
				void onConnectionResumed();
				void onConnectionLost();
				void onConnectionRefused();
				void onConnectionTimedOut();

				void onDatagramSentAcked(Datagram::ID datagramId);
				void onDatagramSentLost(Datagram::ID datagramId);
				void onDatagramReceivedLost(Datagram::ID datagramId);
				void onDataReceived(const uint8* data, uint16 datasize);
				void onMessageReady(std::unique_ptr<Messages::Base>&& msg);

				void fillKeepAlive(Datagram& dgram);
				void handleKeepAlive(const uint8* data, const uint16 datasize);

				void fillDatagramHeader(Datagram& dgram, Datagram::Type type);
				void send(const Datagram& dgram);

			private:
				Client& mClient;
				ChannelsHandler mChannelsHandler;
				Address mAddress;
				uint64 mClientId;
				Datagram::ID mNextDatagramIdToSend{ 0 };
				AckHandler mReceivedAcks;	//!< To detect missing received datagrams and duplicates
				AckHandler mSentAcks;		//!< Which sent datagrams have been acked to detect loss
				std::chrono::milliseconds mConnectionStartTime; // Connection start time, for connection timeout
				std::chrono::milliseconds mLastKeepAlive; // Last time this connection has been marked alive, for timeout disconnection
				static std::chrono::milliseconds sTimeout; // Timeout is same for all clients
				State mState{ State::None };
			#if BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
				bool mInterrupted{ false }; // Whether the connectivity is interrupted with this client (this client stopped sending us data)
				bool mDistantInterrupted{ false }; // Whether this client has its connectivity interrupted with one of its clients
			#endif // BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
				DisconnectionReason mDisconnectionReason{ DisconnectionReason::None };
				std::vector<std::unique_ptr<Messages::Base>> mPendingMessages; // Store messages before connection has been accepted
			};
			
			template<class T>
			void DistantClient::registerChannel()
			{
				mChannelsHandler.registerChannel<T>();
			}
		}
	}
}
