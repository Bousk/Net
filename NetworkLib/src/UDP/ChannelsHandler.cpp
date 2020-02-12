#include <UDP/ChannelsHandler.hpp>
#include <UDP/ChannelHeader.hpp>
#include <UDP/Protocols/UnreliableOrdered.hpp>
#include <UDP/Protocols/ReliableOrdered.hpp>

#include <cassert>
#include <iterator>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			ChannelsHandler::ChannelsHandler() = default;
			ChannelsHandler::~ChannelsHandler() = default;

			// Multiplexer
			void ChannelsHandler::queue(std::vector<uint8>&& msgData, const uint32 channelIndex)
			{
				assert(channelIndex < mChannels.size());
				mChannels[channelIndex]->queue(std::move(msgData));
			}
			size_t ChannelsHandler::serialize(uint8* buffer, const size_t buffersize, const Datagram::ID datagramId)
			{
				size_t remainingBuffersize = buffersize;
				for (uint32_t channelId = 0; channelId < mChannels.size(); ++channelId)
				{
					Protocols::IProtocol* protocol = mChannels[channelId].get();

					uint8_t* const channelHeaderStart = buffer;
					uint8_t* const channelDataStart = buffer + ChannelHeader::Size;
					const size_t channelAvailableSize = remainingBuffersize - ChannelHeader::Size;

					const size_t serializedData = protocol->serialize(channelDataStart, channelAvailableSize, datagramId);
					assert(serializedData <= channelAvailableSize);
					if (serializedData)
					{
						// Data added, let's add the protocol header
						ChannelHeader* const channelHeader = reinterpret_cast<ChannelHeader*>(channelHeaderStart);
						channelHeader->channelId = channelId;
						channelHeader->datasize = static_cast<uint32_t>(serializedData);

						const size_t channelTotalSize = serializedData + ChannelHeader::Size;
						buffer += channelTotalSize;
						remainingBuffersize -= channelTotalSize;
					}
				}
				return buffersize - remainingBuffersize;
			}

			void ChannelsHandler::onDatagramAcked(const Datagram::ID datagramId)
			{
				for (auto& channel : mChannels)
				{
					channel->onDatagramAcked(datagramId);
				}
			}
			void ChannelsHandler::onDatagramLost(const Datagram::ID datagramId)
			{
				for (auto& channel : mChannels)
				{
					channel->onDatagramLost(datagramId);
				}
			}

			// Demultiplexer
			void ChannelsHandler::onDataReceived(const uint8_t* data, const size_t datasize)
			{
				size_t processedData = 0;
				while (processedData < datasize)
				{
					const ChannelHeader* channelHeader = reinterpret_cast<const ChannelHeader*>(data);
					if (processedData + channelHeader->datasize > datasize || channelHeader->datasize > Datagram::DataMaxSize)
					{
						// Malformed buffer
						return;
					}
					if (channelHeader->channelId >= mChannels.size())
					{
						// Channel id requested doesn't exist
						return;
					}
					mChannels[channelHeader->channelId]->onDataReceived(data + ChannelHeader::Size, channelHeader->datasize);
					const size_t channelTotalSize = channelHeader->datasize + ChannelHeader::Size;
					data += channelTotalSize;
					processedData += channelTotalSize;
				}
			}
			std::vector<std::vector<uint8_t>> ChannelsHandler::process(bool isConnected)
			{
				std::vector<std::vector<uint8_t>> messages;
				for (auto& channel : mChannels)
				{
					std::vector<std::vector<uint8_t>> protocolMessages = channel->process();
					if (!protocolMessages.empty() && (channel->isReliable() || isConnected))
					{
						messages.reserve(messages.size() + protocolMessages.size());
						messages.insert(messages.end(), std::make_move_iterator(protocolMessages.begin()), std::make_move_iterator(protocolMessages.end()));
					}
				}
				return messages;
			}
		}
	}
}