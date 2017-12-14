#pragma once

#include "Tester.hpp"
#include "Utils.hpp"

class Utils_Test
{
public:
	static void Test();
};

void Utils_Test::Test()
{
	CHECK(Bousk::Utils::IsSequenceNewer(1, 0));
	CHECK(!Bousk::Utils::IsSequenceNewer(0, 1));
	CHECK(!Bousk::Utils::IsSequenceNewer(0, 0));
	CHECK(Bousk::Utils::IsSequenceNewer(0, std::numeric_limits<uint16_t>::max()));

	CHECK(Bousk::Utils::SequenceDiff(0, 0) == 0);
	CHECK(Bousk::Utils::SequenceDiff(1, 0) == 1);
	CHECK(Bousk::Utils::SequenceDiff(0, std::numeric_limits<uint16_t>::max()) == 1);

	uint64_t bitfield = 0;
	CHECK(bitfield == 0);
	Bousk::Utils::SetBit(bitfield, 0);
	CHECK(Bousk::Utils::HasBit(bitfield, 0));
	CHECK(bitfield == Bousk::Utils::Bit<uint64_t>::Right);
	Bousk::Utils::UnsetBit(bitfield, 0);
	CHECK(bitfield == 0);
	Bousk::Utils::SetBit(bitfield, 5);
	CHECK(Bousk::Utils::HasBit(bitfield, 5));
	CHECK(bitfield == (Bousk::Utils::Bit<uint64_t>::Right << 5));
	Bousk::Utils::UnsetBit(bitfield, 0);
	CHECK(bitfield == (Bousk::Utils::Bit<uint64_t>::Right << 5));
	Bousk::Utils::UnsetBit(bitfield, 5);
	CHECK(bitfield == 0);
}