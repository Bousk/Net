#pragma once

#include "UDP/AckHandler.hpp"
#include "UDP/Packet.hpp"
#include "UDP/Protocols/ProtocolInterface.hpp"

#include <vector>
#include <limits>

class Multiplexer_Test;
class Demultiplexer_Test;
namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			namespace Protocols
			{
				class UnreliableOrdered : public IProtocol
				{
					friend class Multiplexer_Test;
					friend class Demultiplexer_Test;
				public:
					UnreliableOrdered() = default;
					~UnreliableOrdered() override = default;

					void queue(std::vector<uint8_t>&& msgData) override { mMultiplexer.queue(std::move(msgData)); }
					size_t serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID datagramId) override { return mMultiplexer.serialize(buffer, buffersize, datagramId); }

					void onDataReceived(const uint8_t* data, const size_t datasize) override { mDemultiplexer.onDataReceived(data, datasize); }
					std::vector<std::vector<uint8_t>> process() override { return mDemultiplexer.process(); }

				private:
					class Multiplexer
					{
						friend class Multiplexer_Test;
					public:
						Multiplexer() = default;
						~Multiplexer() = default;

						void queue(std::vector<uint8_t>&& msgData);
						size_t serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID);

					private:
						std::vector<Packet> mQueue;
						Packet::Id mNextId{ 0 };
					} mMultiplexer;
					class Demultiplexer
					{
						friend class Demultiplexer_Test;
					public:
						Demultiplexer() = default;
						~Demultiplexer() = default;

						void onDataReceived(const uint8_t* data, const size_t datasize);
						std::vector<std::vector<uint8_t>> process();

					private:
						void onPacketReceived(const Packet* pckt);

					private:
						std::vector<Packet> mPendingQueue;
						Packet::Id mLastProcessed{ std::numeric_limits<Packet::Id>::max() };
					} mDemultiplexer;
				};
			}
		}
	}
}