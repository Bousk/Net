#pragma once

class Multiplexer_Test
{
public:
	static void Test();
};

class Demultiplexer_Test
{
public:
	static void Test();
};

class UnreliableOrdered_Test
{
public:
	static void Test()
	{
		Multiplexer_Test::Test();
		Demultiplexer_Test::Test();
	}
};