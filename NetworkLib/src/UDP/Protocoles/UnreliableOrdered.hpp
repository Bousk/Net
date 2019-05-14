#pragma once

#include "UDP/AckHandler.hpp"
#include "UDP/Packet.hpp"
#include "UDP/Protocoles/ProtocolInterface.hpp"

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
				namespace UnreliableOrdered
				{
					class Multiplexer : public IMultiplexer
					{
						friend class Multiplexer_Test;
					public:
						Multiplexer() = default;
						~Multiplexer() override = default;

						void queue(std::vector<uint8_t>&& msgData) override;
						size_t serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID) override;

					private:
						std::vector<Packet> mQueue;
						Packet::Id mNextId{ 0 };
					};
					class Demultiplexer : public IDemultiplexer
					{
						friend class Demultiplexer_Test;
					public:
						Demultiplexer() = default;
						~Demultiplexer() override = default;

						void onDataReceived(const uint8_t* data, const size_t datasize) override;
						std::vector<std::vector<uint8_t>> process() override;

					private:
						void onPacketReceived(const Packet* pckt);

					private:
						std::vector<Packet> mPendingQueue;
						Packet::Id mLastProcessed{ std::numeric_limits<Packet::Id>::max() };
					};
				}
			}
		}
	}
}