#include "Utils_Test.hpp"
#include "AckHandler_Test.hpp"
#include "DistantClient_Test.hpp"
#include "PacketHandling_Test.hpp"
#include "ReliableOrdered_Test.hpp"

int main()
{
	Utils_Test::Test();
	AckHandler_Test::Test();
	PacketHandling_Test::Test();
	DistantClient_Test::Test();
	ReliableOrdered_Test::Test();
	return 0;
}