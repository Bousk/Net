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
			using HeaderType = uint16;
			static const unsigned int HeaderSize = sizeof(HeaderType);

			class ConnectionHandler;
			class ReceptionHandler;
			class SendingHandler;

			class Client
			{
			public:
				Client();
				Client(const Client&) = delete;
				Client& operator=(const Client&) = delete;
				Client(Client&&) noexcept;
				Client& operator=(Client&&) noexcept;
				~Client();

				bool init(SOCKET&& sckt, const Address& addr);
				bool connect(const Address& address);
				void disconnect();
				bool send(const uint8* data, size_t len);
				std::unique_ptr<Messages::Base> poll();

				inline uint64 id() const { return static_cast<uint64>(mSocket); }
				inline const Address& address() const { return mAddress; }

			private:
				void onConnected(const Address& addr);

			private:
				enum class State {
					Connecting,
					Connected,
					Disconnected,
				};

				std::unique_ptr<ConnectionHandler> mConnectionHandler;
				std::unique_ptr<SendingHandler> mSendingHandler;
				std::unique_ptr<ReceptionHandler> mReceivingHandler;
				Address mAddress;
				SOCKET mSocket{ INVALID_SOCKET };
				State mState{ State::Disconnected };
			};
		}
	}
}