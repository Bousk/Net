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
					using IProtocol::IProtocol;
					~ReliableOrdered() override = default;

					void queue(std::vector<uint8>&& msgData) override { mMultiplexer.queue(std::move(msgData)); }
					uint16 serialize(uint8* buffer, const uint16 buffersize, const Datagram::ID datagramId
					#if BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
						, const bool connectionInterrupted
					#endif // BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
					) override { return mMultiplexer.serialize(buffer, buffersize, datagramId
					#if BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
						, connectionInterrupted
					#endif // BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
					); }

					void onDatagramAcked(const Datagram::ID datagramId) override { mMultiplexer.onDatagramAcked(datagramId); }
					void onDatagramLost(const Datagram::ID datagramId) override { mMultiplexer.onDatagramLost(datagramId); }

					void onDataReceived(const uint8* data, const uint16 datasize) override { mDemultiplexer.onDataReceived(data, datasize); }
					std::vector<std::vector<uint8>> process() override { return mDemultiplexer.process(); }

					virtual bool isReliable() const { return true; }

				private:
					class Multiplexer
					{
						friend class ReliableOrdered_Multiplexer_Test;
					public:
						Multiplexer() = default;
						~Multiplexer() = default;

						void queue(std::vector<uint8>&& msgData);
						uint16 serialize(uint8* buffer, const uint16 buffersize, const Datagram::ID datagramId
						#if BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
							, bool connectionInterrupted
						#endif // BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
						);

						void onDatagramAcked(const Datagram::ID datagramId);
						void onDatagramLost(const Datagram::ID datagramId);

					private:
						class ReliablePacket
						{
						public:
							Packet& packet() { return mPacket; }
							const Packet& packet() const { return mPacket; }

							bool shouldSend() const { return mShouldSend; }
							void resend() { mShouldSend = true; }
							void onSent(const Datagram::ID datagramId) { mDatagramsIncluding.insert(datagramId); mShouldSend = false; }
							bool isIncludedIn(const Datagram::ID datagramId) const { return mDatagramsIncluding.find(datagramId) != mDatagramsIncluding.cend(); }

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

						void onDataReceived(const uint8* data, const uint16 datasize);
						std::vector<std::vector<uint8>> process();

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