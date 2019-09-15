#pragma once

class Serialization_Test
{
public:
	static void Test()
	{
		TestBasics();
		TestBits();
		TestAdvanced();
	}

private:
	static void TestBasics();
	static void TestBits();
	static void TestAdvanced();
};