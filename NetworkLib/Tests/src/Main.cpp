#include "AckHandler_Test.hpp"
#include "DistantClient_Test.hpp"
#include "ReliableOrdered_Test.hpp"
#include "Serialization_Test.hpp"
#include "Types_Test.hpp"
#include "Tester.hpp"
#include "UnreliableOrdered_Test.hpp"
#include "Utils_Test.hpp"

#include <Sockets.hpp>
#include <Types.hpp>

int main()
{
	//int32_t value = 0x01020304;
	//int8_t* ptr = reinterpret_cast<int8_t*>(&value);
	//printf("%p : 0x%08x\n", ptr, value);
	//for (size_t i = 0; i < 4; ++i)
	//	printf("%p : 0x%02x\n", ptr + i, *(ptr + i));

	CHECK(Bousk::Network::Start());

	Utils_Test::Test();
	AckHandler_Test::Test();
	UnreliableOrdered_Test::Test();
	DistantClient_Test::Test();
	ReliableOrdered_Test::Test();
	Serialization_Test::Test();
	Types_Test::Test();

	Bousk::Network::Release();
	return 0;
}