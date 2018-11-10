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