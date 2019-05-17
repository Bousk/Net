#include <UDP/ProtocolsHandler.hpp>
#include <UDP/ProtocolsHeader.hpp>
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
			ProtocolsHandler::ProtocolsHandler()
			{
				mMultiplexers.push_back(std::make_unique<Protocols::UnreliableOrdered::Multiplexer>());
				mMultiplexers.push_back(std::make_unique<Protocols::ReliableOrdered::Multiplexer>());

				mDemultiplexers.push_back(std::make_unique<Protocols::UnreliableOrdered::Demultiplexer>());
				mDemultiplexers.push_back(std::make_unique<Protocols::ReliableOrdered::Demultiplexer>());
			}

			// Multiplexer
			void ProtocolsHandler::queue(std::vector<uint8_t>&& msgData, size_t canalIndex)
			{
				assert(canalIndex < mMultiplexers.size());
				mMultiplexers[canalIndex]->queue(std::move(msgData));
			}
			size_t ProtocolsHandler::serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID datagramId)
			{
				size_t remainingBuffersize = buffersize;
				for (uint32_t protocolid = 0; protocolid < mMultiplexers.size(); ++protocolid)
				{
					Protocols::IMultiplexer* protocol = mMultiplexers[protocolid].get();

					uint8_t* const protocolHeaderStart = buffer;
					uint8_t* const protocolDataStart = buffer + ProtocolHeader::Size;
					const size_t protocolAvailableSize = remainingBuffersize - ProtocolHeader::Size;

					const size_t serializedData = protocol->serialize(protocolDataStart, protocolAvailableSize, datagramId);
					assert(serializedData <= protocolAvailableSize);
					if (serializedData)
					{
						// Data added, let's add the protocol header
						ProtocolHeader* const protocolHeader = reinterpret_cast<ProtocolHeader*>(protocolHeaderStart);
						protocolHeader->canalId = protocolid;
						protocolHeader->datasize = static_cast<uint32_t>(serializedData);

						const size_t protocolTotalSize = serializedData + ProtocolHeader::Size;
						buffer += protocolTotalSize;
						remainingBuffersize -= protocolTotalSize;
					}
				}
				return buffersize - remainingBuffersize;
			}

			void ProtocolsHandler::onDatagramAcked(Datagram::ID datagramId)
			{
				for (auto& protocol : mMultiplexers)
				{
					protocol->onDatagramAcked(datagramId);
				}
			}
			void ProtocolsHandler::onDatagramLost(Datagram::ID datagramId)
			{
				for (auto& protocol : mMultiplexers)
				{
					protocol->onDatagramLost(datagramId);
				}
			}

			// Demultiplexer
			void ProtocolsHandler::onDataReceived(const uint8_t* data, const size_t datasize)
			{
				size_t processedData = 0;
				while (processedData < datasize)
				{
					const ProtocolHeader* protocolHeader = reinterpret_cast<const ProtocolHeader*>(data);
					if (processedData + protocolHeader->datasize > datasize || protocolHeader->datasize > Datagram::DataMaxSize)
					{
						// Malformed buffer
						return;
					}
					if (protocolHeader->canalId >= mDemultiplexers.size())
					{
						// Canal id requested doesn't exist
						return;
					}
					mDemultiplexers[protocolHeader->canalId]->onDataReceived(data + ProtocolHeader::Size, protocolHeader->datasize);
					const size_t protocolTotalSize = protocolHeader->datasize + ProtocolHeader::Size;
					data += protocolTotalSize;
					processedData += protocolTotalSize;
				}
			}
			std::vector<std::vector<uint8_t>> ProtocolsHandler::process()
			{
				std::vector<std::vector<uint8_t>> messages;
				for (auto& protocol : mDemultiplexers)
				{
					std::vector<std::vector<uint8_t>> protocolMessages = protocol->process();
					if (!protocolMessages.empty())
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