#include "Utils_Test.hpp"
#include "AckHandler_Test.hpp"
#include "DistantClient_Test.hpp"

int main()
{
	Utils_Test::Test();
	AckHandler_Test::Test();
	DistantClient_Test::Test();
	return 0;
}