#pragma once

class ReliableOrdered_Multiplexer_Test
{
public:
	static void Test();
};

class ReliableOrdered_Demultiplexer_Test
{
public:
	static void Test();
};

class ReliableOrdered_Test
{
public:
	static void Test()
	{
		ReliableOrdered_Multiplexer_Test::Test();
		ReliableOrdered_Demultiplexer_Test::Test();
	}
};