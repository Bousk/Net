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

			static Address Any(Type type, uint16 port);

			inline bool isValid() const { return mType != Type::None; }
			inline Type type() const { return mType; }
			inline uint16 port() const { return mPort; }
			std::string toString() const;

			bool operator==(const Address& other) const;

			// Connect the given socket to the internal address
			int connect(SOCKET sckt) const;
			// Accept an incoming connection on the given socket then update the internal address with the sender one
			// Returns true if a new client has been accepted and set its socket value to newClient. False otherwise
			bool accept(SOCKET sckt, SOCKET& newClient);
			// Bind the given socket to the internal address
			int bind(SOCKET sckt) const;
			// Send data from the given socket to the internal address
			int sendTo(SOCKET sckt, const char* data, size_t datalen) const;
			// Receive data from the given socket then update the internal address with the sender one
			int recvFrom(SOCKET sckt, uint8* buffer, size_t bufferSize);

		private:
			void set(const sockaddr_storage& src);

		private:
			sockaddr_storage mStorage{ 0 };
			uint16 mPort{ 0 };
			Type mType{ Type::None };
		};
	}
}