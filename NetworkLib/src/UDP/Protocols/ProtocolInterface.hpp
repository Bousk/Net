#pragma once

#include <Settings.hpp>
#include <UDP/Datagram.hpp>

#include <optional>
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
					IProtocol(uint8 channelId)
						: mChannelId(channelId)
					{}
					virtual ~IProtocol() = default;

					uint8 channelId() const { return mChannelId; }

					virtual void queue(std::vector<uint8>&& msgData) = 0;
					virtual uint16 serialize(uint8* buffer, uint16 buffersize, Datagram::ID datagramId
					#if BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
						, bool connectionInterrupted
					#endif // BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
					) = 0;

					virtual void onDatagramAcked(Datagram::ID /*datagramId*/) {}
					virtual void onDatagramLost(Datagram::ID /*datagramId*/) {}

					virtual void onDataReceived(const uint8* data, uint16 datasize) = 0;
					virtual std::vector<std::vector<uint8>> process() = 0;

					virtual bool isReliable() const = 0;

				private:
					uint8 mChannelId;
				};
			}
		}
	}
}