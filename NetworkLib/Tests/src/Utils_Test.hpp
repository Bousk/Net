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
	CHECK(Bousk::Utils::CountNeededBits(1) == 1);
	CHECK(Bousk::Utils::CountNeededBits(2) == 2);
	CHECK(Bousk::Utils::CountNeededBits(3) == 2);
	CHECK(Bousk::Utils::CountNeededBits(4) == 3);
	CHECK(Bousk::Utils::CountNeededBits(5) == 3);
	CHECK(Bousk::Utils::CountNeededBits(8) == 4);
	CHECK(Bousk::Utils::CountNeededBits(12) == 4);
	CHECK(Bousk::Utils::CountNeededBits(16) == 5);
	CHECK(Bousk::Utils::CountNeededBits(127) == 7);
	CHECK(Bousk::Utils::CountNeededBits(255) == 8);
	CHECK(Bousk::Utils::CountNeededBits(std::numeric_limits<Bousk::uint16>::max()) == 16);
	CHECK(Bousk::Utils::CountNeededBits(std::numeric_limits<Bousk::uint16>::max() / 2) == 15);
	CHECK(Bousk::Utils::CountNeededBits(std::numeric_limits<Bousk::uint16>::max() / 2 + 1) == 16);
	CHECK(Bousk::Utils::CountNeededBits(std::numeric_limits<Bousk::uint32>::max()) == 32);
	CHECK(Bousk::Utils::CountNeededBits(std::numeric_limits<Bousk::uint32>::max() / 2) == 31);
	CHECK(Bousk::Utils::CountNeededBits(std::numeric_limits<Bousk::uint32>::max() / 2 + 1) == 32);
	CHECK(Bousk::Utils::CountNeededBits(std::numeric_limits<Bousk::uint64>::max()) == 64);
	CHECK(Bousk::Utils::CountNeededBits(std::numeric_limits<Bousk::uint64>::max() / 2) == 63);
	CHECK(Bousk::Utils::CountNeededBits(std::numeric_limits<Bousk::uint64>::max() / 2 + 1) == 64);
	CHECK(Bousk::Utils::CreateBitsMask(3, 2) == 0b00011100);
}