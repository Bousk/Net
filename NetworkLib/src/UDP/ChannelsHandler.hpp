#pragma once

#include <UDP/Datagram.hpp>

#include <memory>
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
				void registerChannel();

				// Multiplexer
				void queue(std::vector<uint8>&& msgData, uint32 channelIndex);
				size_t serialize(uint8* buffer, size_t buffersize, Datagram::ID datagramId);

				void onDatagramAcked(Datagram::ID datagramId);
				void onDatagramLost(Datagram::ID datagramId);

				// Demultiplexer
				void onDataReceived(const uint8_t* data, size_t datasize);
				std::vector<std::vector<uint8_t>> process(bool isConnected);

			private:
				std::vector<std::unique_ptr<Protocols::IProtocol>> mChannels;
			};

			template<class T>
			void ChannelsHandler::registerChannel()
			{
				mChannels.push_back(std::make_unique<T>());
			}
		}
	}
}