#pragma once

#include "UDP/AckHandler.hpp"
#include "UDP/Packet.hpp"
#include "UDP/Protocoles/ProtocolInterface.hpp"

#include <vector>
#include <set>
#include <limits>

class ReliableOrdered_Multiplexer_Test;
class ReliableOrdered_Demultiplexer_Test;
namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			namespace Protocols
			{
				namespace ReliableOrdered
				{
					class Multiplexer : public IMultiplexer
					{
						friend class ReliableOrdered_Multiplexer_Test;
					public:
						Multiplexer() = default;
						~Multiplexer() override = default;

						void queue(std::vector<uint8_t>&& msgData) override;
						size_t serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID datagramId) override;

						void onDatagramAcked(Datagram::ID datagramId) override;
						void onDatagramLost(Datagram::ID datagramId) override;

					private:
						class ReliablePacket
						{
						public:
							Packet& packet() { return mPacket; }
							const Packet& packet() const { return mPacket; }

							bool shouldSend() const { return mShouldSend; }
							void resend() { mShouldSend = true; }
							void onSent(Datagram::ID datagramId) { mDatagramsIncluding.insert(datagramId); mShouldSend = false; }
							bool isIncludedIn(Datagram::ID datagramId) const { return mDatagramsIncluding.find(datagramId) != mDatagramsIncluding.cend(); }

						private:
							Packet mPacket;
							std::set<Datagram::ID> mDatagramsIncluding;
							bool mShouldSend{ true };
						};
						std::vector<ReliablePacket> mQueue;
						Packet::Id mNextId{ 0 };
					};

					class Demultiplexer : public IDemultiplexer
					{
						friend class ReliableOrdered_Demultiplexer_Test;
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