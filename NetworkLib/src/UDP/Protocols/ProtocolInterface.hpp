#pragma once

#include <UDP/Datagram.hpp>
#include <vector>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			namespace Protocols
			{
				class IProtocol
				{
				public:
					IProtocol() = default;
					virtual ~IProtocol() = default;

					virtual void queue(std::vector<uint8_t>&& msgData) = 0;
					virtual size_t serialize(uint8_t* buffer, size_t buffersize, Datagram::ID datagramId) = 0;

					virtual void onDatagramAcked(Datagram::ID /*datagramId*/) {}
					virtual void onDatagramLost(Datagram::ID /*datagramId*/) {}

					virtual void onDataReceived(const uint8_t* data, size_t datasize) = 0;
					virtual std::vector<std::vector<uint8_t>> process() = 0;
				};
			}
		}
	}
}