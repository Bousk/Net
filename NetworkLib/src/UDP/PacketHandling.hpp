#pragma once

#include "UDP/Packet.hpp"
#include "UDP/AckHandler.hpp"
#include <vector>

class Multiplexer_Test;
namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			class Multiplexer
			{
				friend class Multiplexer_Test;
			public:
				Multiplexer() = default;
				~Multiplexer() = default;

				void queue(std::vector<uint8_t>&& data);
				size_t serialize(uint8_t* buffer, const size_t buffersize);

			private:
				std::vector<Packet> mQueue;
				Packet::Id mNextId{ 0 };
			};
			class Demultiplexer
			{
			public:
				Demultiplexer() = default;
				~Demultiplexer() = default;

				void onDataReceived(const uint8_t* data, const size_t datasize);
				std::vector<std::vector<uint8_t>> process();

			private:
				void queue(const Packet* pckt);

			private:
				std::vector<Packet> mPendingQueue;
				AckHandler mAcceptor;
				Packet::Id mLastProcessed{ std::numeric_limits<Packet::Id>::max() };
			};
		}
	}
}