#include "Address.hpp"
#include "Serialization/Convert.hpp"

#include <algorithm>

namespace Bousk
{
	namespace Network
	{
		Address::Address(const Address& src)
			: mPort(src.mPort)
			, mType(src.mType)
		{
			memcpy(&mStorage, &(src.mStorage), sizeof(mStorage));
		}
		Address::Address(Address&& src)
			: mPort(src.mPort)
			, mType(src.mType)
		{
			memcpy(&mStorage, &(src.mStorage), sizeof(mStorage));
		}
		Address& Address::operator=(const Address& src)
		{
			mPort = src.mPort;
			mType = src.mType;
			memcpy(&mStorage, &(src.mStorage), sizeof(mStorage));
			return *this;
		}
		Address& Address::operator=(Address&& src)
		{
			mPort = src.mPort;
			mType = src.mType;
			memcpy(&mStorage, &(src.mStorage), sizeof(mStorage));
			return *this;
		}

		Address::Address(const std::string& ip, uint16_t port)
			: mPort(port)
		{
			memset(&mStorage, 0, sizeof(mStorage));
			if (!ip.empty())
			{
				// IpV4 ?
				{
					sockaddr_in& addrin = reinterpret_cast<sockaddr_in&>(mStorage);
					in_addr& inaddr = addrin.sin_addr;
					if (inet_pton(AF_INET, ip.c_str(), &inaddr) == 1)
					{
						mType = Type::IPv4;
						addrin.sin_family = AF_INET;
						Serialization::Conversion::ToNetwork(mPort, addrin.sin_port);
						return;
					}
				}
				// IpV6 ?
				{
					sockaddr_in6& addrin = reinterpret_cast<sockaddr_in6&>(mStorage);
					in_addr6& inaddr = addrin.sin6_addr;
					if (inet_pton(AF_INET6, ip.c_str(), &inaddr) == 1)
					{
						mType = Type::IPv6;
						addrin.sin6_family = AF_INET6;
						Serialization::Conversion::ToNetwork(mPort, addrin.sin6_port);
						return;
					}
				}
			}
		}
		Address::Address(const sockaddr_storage& addr)
		{
			set(addr);
		}
		void Address::set(const sockaddr_storage& src)
		{
			memcpy(&mStorage, &src, sizeof(mStorage));
			if (mStorage.ss_family == AF_INET)
			{
				mType = Type::IPv4;
				const sockaddr_in& addrin = reinterpret_cast<const sockaddr_in&>(mStorage);
				Serialization::Conversion::ToLocal(addrin.sin_port, mPort);
			}
			else if (mStorage.ss_family == AF_INET6)
			{
				mType = Type::IPv6;
				const sockaddr_in6& addrin = reinterpret_cast<const sockaddr_in6&>(mStorage);
				Serialization::Conversion::ToLocal(addrin.sin6_port, mPort);
			}
			else
				mType = Type::None;
		}

		Address Address::Any(Type type, uint16 port)
		{
			switch (type)
			{
			case Type::IPv4:
			{
				sockaddr_in addr = { 0 };
				addr.sin_addr.s_addr = INADDR_ANY;
				addr.sin_port = htons(port);
				addr.sin_family = AF_INET;
				return Address(reinterpret_cast<sockaddr_storage&>(addr));
			}
			case Type::IPv6:
			{
				sockaddr_in6 addr = { 0 };
				addr.sin6_addr = in6addr_any;
				addr.sin6_port = htons(port);
				addr.sin6_family = AF_INET6;
				return Address(reinterpret_cast<sockaddr_storage&>(addr));
			}
			default:
				assert(false);
				return Address();
			}
		}

		std::string Address::toString() const
		{
			if (mType == Type::None)
				return "";
			static constexpr int MaxBufferSize = std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN);
			char buffer[MaxBufferSize];
			// Use a const_cast because of some Windows API... the object is not changed so it's okay
			if (inet_ntop(mStorage.ss_family, const_cast<sockaddr_storage*>(&mStorage), buffer, MaxBufferSize) != nullptr)
				return buffer;
			return "";
		}

		bool Address::operator==(const Address& other) const
		{
			if (mType != other.mType)
				return false;
			if (mType == Type::None)
				return true;
			if (mPort != other.mPort)
				return false;
			if (mType == Type::IPv4)
			{
				return memcmp(&mStorage, &(other.mStorage), sizeof(mStorage)) == 0;
			}
			// IpV6
			return memcmp(&reinterpret_cast<const sockaddr_in6&>(mStorage).sin6_addr, &reinterpret_cast<const sockaddr_in6&>(other.mStorage).sin6_addr, sizeof(IN6_ADDR)) == 0;
		}

		int Address::connect(SOCKET sckt) const
		{
			return ::connect(sckt, reinterpret_cast<const sockaddr*>(&mStorage), sizeof(mStorage));
		}
		bool Address::accept(SOCKET sckt, SOCKET& newClient)
		{
			sockaddr_in addr = { 0 };
			socklen_t addrlen = sizeof(addr);
			SOCKET newClientSocket = ::accept(sckt, reinterpret_cast<sockaddr*>(&addr), &addrlen);
			if (newClientSocket == INVALID_SOCKET)
			{
				return false;
			}
			set(reinterpret_cast<sockaddr_storage&>(addr));
			newClient = newClientSocket;
			return true;
		}
		bool Address::bind(SOCKET sckt) const
		{
			return ::bind(sckt, reinterpret_cast<const sockaddr*>(&mStorage), sizeof(mStorage)) == 0;
		}
		int Address::sendTo(SOCKET sckt, const char* data, size_t datalen) const
		{
			return sendto(sckt, data, static_cast<int>(datalen), 0, reinterpret_cast<const sockaddr*>(&mStorage), sizeof(mStorage));
		}
		int Address::recvFrom(SOCKET sckt, uint8* buffer, size_t bufferSize)
		{
			sockaddr_in from{ 0 };
			socklen_t fromlen = sizeof(from);
			int ret = recvfrom(sckt, reinterpret_cast<char*>(buffer), static_cast<int>(bufferSize), 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
			if (ret >= 0)
			{
				set(reinterpret_cast<sockaddr_storage&>(from));
			}
			return ret;
		}
	}
}