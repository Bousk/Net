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

				void queue(std::vector<unsigned char>&& data);
				size_t serialize(unsigned char* buffer, const size_t buffersize);

			private:
				std::vector<Packet> mQueue;
				Packet::Id mNextId{ 0 };
			};
			class Demultiplexer
			{
			public:
				Demultiplexer() = default;
				~Demultiplexer() = default;

				void queue(Packet&& pckt);
				std::vector<std::vector<unsigned char>> process();

			private:
				std::vector<Packet> mPendingQueue;
				AckHandler mAcceptor;
				Packet::Id mLastProcessed{ std::numeric_limits<Packet::Id>::max() };
			};
		}
	}
}