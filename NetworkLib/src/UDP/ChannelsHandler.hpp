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

				// Multiplexer
				void queue(std::vector<uint8_t>&& msgData, uint32_t canalIndex);
				size_t serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID datagramId);

				void onDatagramAcked(Datagram::ID datagramId);
				void onDatagramLost(Datagram::ID datagramId);

				// Demultiplexer
				void onDataReceived(const uint8_t* data, const size_t datasize);
				std::vector<std::vector<uint8_t>> process();

			private:
				std::vector<std::unique_ptr<Protocols::IProtocol>> mChannels;
			};
		}
	}
}