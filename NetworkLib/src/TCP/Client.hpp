#pragma once

#include <Address.hpp>
#include <Sockets.hpp>
#include <Types.hpp>

#include <string>
#include <memory>

namespace Bousk
{
	namespace Network
	{
		class Address;
		namespace Messages {
			class Base;
		}
		namespace TCP
		{
			constexpr uint8 TCPChannelId = 0;

			using HeaderType = uint16;
			static constexpr unsigned int HeaderSize = sizeof(HeaderType);

			class ConnectionHandler;
			class ReceptionHandler;
			class SendingHandler;
			class Server;

			class Client
			{
				friend class Server;
			public:
				Client();
				Client(const Client&) = delete;
				Client& operator=(const Client&) = delete;
				Client(Client&&) noexcept;
				Client& operator=(Client&&) noexcept;
				~Client();

				bool connect(const Address& address);
				void accept();
				void disconnect();
				bool send(const uint8* data, size_t len);
				// Extract a message ready.
				// Be careful if you want to loop as long as you poll anything to not lock your app in a receiving loop from a spammy user.
				std::unique_ptr<Messages::Base> poll();

				inline uint64 id() const { return static_cast<uint64>(mSocket); }
				inline const Address& address() const { return mAddress; }

			private:
				// Init client with data from the server
				bool init(SOCKET&& sckt, const Address& addr);

				void onWaitingConnectionToBeAccepted(const Address& addr);
				void onConnectionConfirmed();

			private:
				enum class State {
					Connecting,
					WaitingConnectionToBeAccepted,
					Connected,
					Disconnected,
				};

				std::unique_ptr<ConnectionHandler> mConnectionHandler;
				std::unique_ptr<SendingHandler> mSendingHandler;
				std::unique_ptr<ReceptionHandler> mReceivingHandler;
				Address mAddress;
				SOCKET mSocket{ INVALID_SOCKET };
				State mState{ State::Disconnected };
				// When this is true, this client is a client representation on a server. Used internally and on server only.
				bool mIsServerClient{ false };
			};
		}
	}
}