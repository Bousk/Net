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
						addrin.sin6_family = AF_INET6;
						Serialization::Conversion::ToNetwork(mPort, addrin.sin6_port);
						return;
					}
				}
			}
		}
		Address::Address(const sockaddr_storage& addr)
		{
			memcpy(&mStorage, &addr, sizeof(mStorage));
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

		std::string Address::ToString() const
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

		int Address::sendTo(SOCKET sckt, const char* data, size_t datalen) const
		{
			return sendto(sckt, data, static_cast<int>(datalen), 0, reinterpret_cast<const sockaddr*>(&mStorage), sizeof(mStorage));
		}
	}
}