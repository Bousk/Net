#pragma once

#include <Tester.hpp>
#include <Types.hpp>

class Types_Test
{
public:
	static void Test()
	{
		CHECK(Bousk::Uint8<0, 255>::Min() == 0);
		CHECK(Bousk::Uint8<0, 255>::Max() == 255);
		//Bousk::Uint8<0, 352> impossibleVariable;

		CHECK(Bousk::NbBits<1>::Value == 1);
		CHECK(Bousk::NbBits<3>::Value == 2);
		CHECK(Bousk::NbBits<4>::Value == 3);
		CHECK(Bousk::NbBits<5>::Value == 3);
		CHECK(Bousk::NbBits<8>::Value == 4);
		CHECK(Bousk::NbBits<12>::Value == 4);
		CHECK(Bousk::NbBits<16>::Value == 5);
		CHECK(Bousk::NbBits<127>::Value == 7);
		CHECK(Bousk::NbBits<255>::Value == 8);
		CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint16>::max()>::Value == 16);
		CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint16>::max()/2>::Value == 15);
		CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint16>::max()/2 + 1>::Value == 16);
		CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint32>::max()>::Value == 32);
		CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint64>::max()>::Value == 64);
	}
};