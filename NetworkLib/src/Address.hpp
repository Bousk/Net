#pragma once

#include "Sockets.hpp"
#include "Types.hpp"

#include <string>

namespace Bousk
{
	namespace Network
	{
		class Address
		{
		public:
			enum class Type {
				None,
				IPv4,
				IPv6,
			};
		public:
			Address() = default;
			Address(const Address&);
			Address(Address&&);
			Address& operator=(const Address&);
			Address& operator=(Address&&);
			~Address() = default;

			Address(const std::string& ip, uint16_t port);
			Address(const sockaddr_storage& addr);

			Type GetType() const { return mType; }
			uint16 GetPort() const { return mPort; }
			std::string ToString() const;

			bool operator==(const Address& other) const;

			int sendTo(SOCKET sckt, const char* data, size_t datalen) const;

		private:
			sockaddr_storage mStorage{ 0 };
			uint16 mPort{ 0 };
			Type mType{ Type::None };
		};
	}
}