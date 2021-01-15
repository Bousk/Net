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
			uint16 ChannelsHandler::serialize(uint8* buffer, const uint16 buffersize, const Datagram::ID datagramId, const bool connectionInterrupted)
			{
				uint16 remainingBuffersize = buffersize;
				for (uint32 channelId = 0; channelId < mChannels.size(); ++channelId)
				{
					Protocols::IProtocol* protocol = mChannels[channelId].get();

					uint8* const channelHeaderStart = buffer;
					uint8* const channelDataStart = buffer + ChannelHeader::Size;
					const uint16 channelAvailableSize = remainingBuffersize - ChannelHeader::Size;

					const uint16 serializedData = protocol->serialize(channelDataStart, channelAvailableSize, datagramId, connectionInterrupted);
					assert(serializedData <= channelAvailableSize);
					if (serializedData)
					{
						// Data added, let's add the protocol header
						ChannelHeader* const channelHeader = reinterpret_cast<ChannelHeader*>(channelHeaderStart);
						channelHeader->channelId = channelId;
						channelHeader->datasize = static_cast<uint32>(serializedData);

						const uint16 channelTotalSize = serializedData + ChannelHeader::Size;
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
			void ChannelsHandler::onDataReceived(const uint8* data, const uint16 datasize)
			{
				uint16 processedData = 0;
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
					const uint16 channelTotalSize = channelHeader->datasize + ChannelHeader::Size;
					data += channelTotalSize;
					processedData += channelTotalSize;
				}
			}
			std::vector<std::vector<uint8>> ChannelsHandler::process(bool isConnected)
			{
				std::vector<std::vector<uint8>> messages;
				for (auto& channel : mChannels)
				{
					std::vector<std::vector<uint8>> protocolMessages = channel->process();
					// If we're not connected, ignore and discard unreliable messages
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