#pragma once

#include "Tester.hpp"
#include "UDP/PacketHandling.hpp"

#include <limits>
#include <chrono>
#include <random>

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
	// Use the multiplexer to easily queue and split data
	// It's been tested before so it's reliable
	Bousk::Network::UDP::Multiplexer mux;
	Bousk::Network::UDP::Demultiplexer demux;
	CHECK(demux.mPendingQueue.empty());
	CHECK(demux.mLastProcessed == std::numeric_limits<Bousk::Network::UDP::Packet::Id>::max());
	{
		std::array<uint8_t, 5> arr0{ 'T', 'o', 't', 'o', '\0' };
		std::vector<uint8_t> data0(arr0.cbegin(), arr0.cend());
		// Receive packet 0 & 1
		{
			mux.queue(std::move(data0));
			Bousk::Network::UDP::Packet packet;
			mux.serialize(reinterpret_cast<uint8_t*>(&packet), Bousk::Network::UDP::Packet::PacketMaxSize);
			CHECK(packet.id() == 0);
			CHECK(packet.type() == Bousk::Network::UDP::Packet::Type::Packet);
			CHECK(packet.datasize() == static_cast<uint16_t>(arr0.size()));
			CHECK(memcmp(packet.data(), arr0.data(), arr0.size()) == 0);
			demux.onDataReceived(packet.buffer(), packet.size());
		}
		std::array<uint8_t, 5> arr1{ 'T', 'a', 't', 'a', '\0' };
		std::vector<uint8_t> data1(arr1.cbegin(), arr1.cend());
		{
			mux.queue(std::move(data1));
			Bousk::Network::UDP::Packet packet;
			mux.serialize(reinterpret_cast<uint8_t*>(&packet), Bousk::Network::UDP::Packet::PacketMaxSize);
			CHECK(packet.id() == 1);
			CHECK(packet.type() == Bousk::Network::UDP::Packet::Type::Packet);
			CHECK(packet.datasize() == static_cast<uint16_t>(arr1.size()));
			CHECK(memcmp(packet.data(), arr1.data(), arr1.size()) == 0);
			demux.onDataReceived(packet.buffer(), packet.size());
		}
		CHECK(demux.mPendingQueue.size() == 2);
		std::vector<std::vector<uint8_t>> packets = demux.process();
		CHECK(packets.size() == 2);
		CHECK(demux.mLastProcessed == 1);
		CHECK(packets[0].size() == data0.size());
		CHECK(packets[0] == data0);
		CHECK(packets[1].size() == data1.size());
		CHECK(packets[1] == data1);
	}
	{
		// Receive fragmented message : 3 fragments
		std::vector<uint8_t> data(Bousk::Network::UDP::Packet::DataMaxSize * 3, 0);
		const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
		std::default_random_engine generator(seed);
		std::uniform_int_distribution<unsigned int> distribution(0, 100);
		for (uint8_t& d : data)
			d = distribution(generator);

		const auto datacopy = data;
		mux.queue(std::move(data));
	}
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