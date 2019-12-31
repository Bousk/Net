#pragma once

#include "Sockets.hpp"
#include "Types.hpp"

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
			using HeaderType = uint16_t;
			static const unsigned int HeaderSize = sizeof(HeaderType);
			class ClientImpl;
			class Client
			{
			public:
				Client();
				Client(const Client&) = delete;
				Client& operator=(const Client&) = delete;
				Client(Client&&);
				Client& operator=(Client&&);
				~Client();

				bool init(SOCKET&& sckt, const Address& addr);
				bool connect(const Address& address);
				void disconnect();
				bool send(const unsigned char* data, unsigned int len);
				std::unique_ptr<Messages::Base> poll();

				uint64 id() const;
				const Address& address() const;

			private:
				std::unique_ptr<ClientImpl> mImpl;
			};
		}
	}
}