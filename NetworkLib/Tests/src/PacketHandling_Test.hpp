#pragma once

#include "Tester.hpp"
#include "UDP/PacketHandling.hpp"

class Multiplexer_Test
{
public:
	static void Test();
};

void Multiplexer_Test::Test()
{
	Bousk::Network::UDP::Multiplexer mux;
	CHECK(mux.mQueue.size() == 0);
	CHECK(mux.mNextId == 0);
	{
		std::array<uint8_t, 5> arr{ 'T', 'o', 't', 'o', '\0' };
		std::vector<uint8_t> data(arr.cbegin(), arr.cend());
		mux.queue(std::move(data));
		CHECK(mux.mQueue.size() == 1);
		CHECK(mux.mNextId == 1);

		std::array<uint8_t, Bousk::Network::UDP::Packet::PacketMaxSize> buffer;
		size_t serializedData = mux.serialize(buffer.data(), buffer.size());
		CHECK(serializedData == Bousk::Network::UDP::Packet::HeaderSize + arr.size());
		const Bousk::Network::UDP::Packet* packet = reinterpret_cast<const Bousk::Network::UDP::Packet*>(buffer.data());
		CHECK(packet->datasize() == arr.size());
		CHECK(memcmp(packet->data(), arr.data(), packet->datasize()) == 0);
	}
}

class Demultiplexer_Test
{
public:
	static void Test();
};

void Demultiplexer_Test::Test()
{
	Bousk::Network::UDP::Demultiplexer demux;
}

class PacketHandling_Test
{
public:
	static void Test()
	{
		Multiplexer_Test::Test();
		Demultiplexer_Test::Test();
	}
};