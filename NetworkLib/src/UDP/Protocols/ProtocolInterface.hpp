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

					virtual void queue(std::vector<uint8>&& msgData) = 0;
					virtual uint16 serialize(uint8* buffer, uint16 buffersize, Datagram::ID datagramId) = 0;

					virtual void onDatagramAcked(Datagram::ID /*datagramId*/) {}
					virtual void onDatagramLost(Datagram::ID /*datagramId*/) {}

					virtual void onDataReceived(const uint8* data, uint16 datasize) = 0;
					virtual std::vector<std::vector<uint8>> process() = 0;

					virtual bool isReliable() const = 0;
				};
			}
		}
	}
}