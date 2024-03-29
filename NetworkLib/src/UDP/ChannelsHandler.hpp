#pragma once

#include <Settings.hpp>
#include <UDP/Datagram.hpp>

#include <memory>
#include <optional>
#include <tuple>
#include <vector>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			namespace Protocols
			{
				class IProtocol;
			}

			class ChannelsHandler
			{
			public:
				ChannelsHandler();
				~ChannelsHandler();

				template<class T>
				void registerChannel(uint8 channelId);

				// Multiplexer
				void queue(std::vector<uint8>&& msgData, uint32 channelIndex);
				uint16 serialize(uint8* buffer, uint16 buffersize, Datagram::ID datagramId
				#if BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
					, bool connectionInterrupted
				#endif // BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
				);

				void onDatagramAcked(Datagram::ID datagramId);
				void onDatagramLost(Datagram::ID datagramId);

				// Demultiplexer
				void onDataReceived(const uint8* data, uint16 datasize);
				std::vector<std::tuple<uint8/*ChannelId*/, std::vector<uint8>>> process(bool isConnected);

			private:
				std::vector<std::unique_ptr<Protocols::IProtocol>> mChannels;
			};

			template<class T>
			void ChannelsHandler::registerChannel(uint8 channelId)
			{
				mChannels.push_back(std::make_unique<T>(channelId));
			}
		}
	}
}