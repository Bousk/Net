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

					void queue(std::vector<uint8>&& msgData) override { mMultiplexer.queue(std::move(msgData)); }
					uint16 serialize(uint8* buffer, uint16 buffersize, Datagram::ID datagramId, bool /*connectionInterrupted*/) override { return mMultiplexer.serialize(buffer, buffersize, datagramId); }

					void onDataReceived(const uint8* data, uint16 datasize) override { mDemultiplexer.onDataReceived(data, datasize); }
					std::vector<std::vector<uint8>> process() override { return mDemultiplexer.process(); }

					virtual bool isReliable() const { return false; }

				private:
					class Multiplexer
					{
						friend class Multiplexer_Test;
					public:
						Multiplexer() = default;
						~Multiplexer() = default;

						void queue(std::vector<uint8>&& msgData);
						uint16 serialize(uint8* buffer, uint16 buffersize, Datagram::ID);

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

						void onDataReceived(const uint8* data, uint16 datasize);
						std::vector<std::vector<uint8>> process();

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