#pragma once

#include "Sockets.hpp"

#include <string>
#include <memory>

namespace Network
{
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
				Client(Client&&) = default;
				Client& operator=(Client&&) = default;
				~Client();

				bool init(SOCKET sckt);
				bool connect(const std::string& ipaddress, unsigned short port);
				void disconnect();
				bool send(const unsigned char* data, unsigned int len);
				std::unique_ptr<Messages::Base> poll();

			private:
				std::unique_ptr<ClientImpl> mImpl;
		};
	}
}