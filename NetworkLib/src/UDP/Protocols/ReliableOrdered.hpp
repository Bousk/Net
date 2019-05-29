#pragma once

#include "UDP/AckHandler.hpp"
#include "UDP/Packet.hpp"
#include "UDP/Protocols/ProtocolInterface.hpp"

#include <vector>
#include <array>
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
				class ReliableOrdered : public IProtocol
				{
					friend class ReliableOrdered_Multiplexer_Test;
					friend class ReliableOrdered_Demultiplexer_Test;
				public:
					ReliableOrdered() = default;
					~ReliableOrdered() override = default;

					void queue(std::vector<uint8_t>&& msgData) override { mMultiplexer.queue(std::move(msgData)); }
					size_t serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID datagramId) override { return mMultiplexer.serialize(buffer, buffersize, datagramId); }

					void onDatagramAcked(Datagram::ID datagramId) override { mMultiplexer.onDatagramAcked(datagramId); }
					void onDatagramLost(Datagram::ID datagramId) override { mMultiplexer.onDatagramLost(datagramId); }

					void onDataReceived(const uint8_t* data, const size_t datasize) override { mDemultiplexer.onDataReceived(data, datasize); }
					std::vector<std::vector<uint8_t>> process() override { return mDemultiplexer.process(); }

				private:
					class Multiplexer
					{
						friend class ReliableOrdered_Multiplexer_Test;
					public:
						Multiplexer() = default;
						~Multiplexer() = default;

						void queue(std::vector<uint8_t>&& msgData);
						size_t serialize(uint8_t* buffer, const size_t buffersize, Datagram::ID datagramId);

						void onDatagramAcked(Datagram::ID datagramId);
						void onDatagramLost(Datagram::ID datagramId);

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
						Packet::Id mFirstAllowedPacket{ 0 };
					} mMultiplexer;
					class Demultiplexer
					{
						friend class ReliableOrdered_Demultiplexer_Test;
					public:
						static constexpr size_t QueueSize = 2 * Packet::MaxPacketsPerMessage;
					public:
						Demultiplexer() = default;
						~Demultiplexer() = default;

						void onDataReceived(const uint8_t* data, const size_t datasize);
						std::vector<std::vector<uint8_t>> process();

					private:
						void onPacketReceived(const Packet* pckt);

					private:
						std::array<Packet, QueueSize> mPendingQueue;
						Packet::Id mLastProcessed{ std::numeric_limits<Packet::Id>::max() };
					} mDemultiplexer;
				};
			}
		}
	}
}