#include <Types_Test.hpp>
#include <Tester.hpp>
#include <Types.hpp>
#include <RangedInteger.hpp>
#include <Float.hpp>

void Types_Test::Test()
{
	CHECK(Bousk::UInt8<0, 255>::Min() == 0);
	CHECK(Bousk::UInt8<0, 255>::Max() == 255);
	//Bousk::UInt8<0, 352> impossibleVariable; // Compile error if uncommented
	STATIC_CHECK(!Bousk::UInt8<0, 42>::IsWithinRange(51));
	//STATIC_CHECK(Bousk::UInt8<0, 42>::IsWithinRange(-8)); // Compile error if uncommented
	STATIC_CHECK(Bousk::UInt8<>::Min() == std::numeric_limits<Bousk::uint8>::min());
	STATIC_CHECK(Bousk::UInt8<>::Max() == std::numeric_limits<Bousk::uint8>::max());

	{
		Bousk::UInt8<0, 42> var;
		var = 13;
		//var = 51; // Runtime error
		Bousk::UInt8<> u8;
		u8 = 66;
	}
	{
		using namespace Bousk::Literals;
		Bousk::int8 value1 = -8_i8;
		CHECK(value1 == -8);
		STATIC_CHECK(-42_i8 == -42);
		Bousk::UInt16<42, 666> value2;
		//value2 = value1; // Runtime error
		//Bousk::int32 value3 = 12345;
		//value2 = value3; // Runtime error
		Bousk::int32 value4 = 55;
		value2 = value4;
	}

	STATIC_CHECK(Bousk::NbBits<1>::Value == 1);
	STATIC_CHECK(Bousk::NbBits<3>::Value == 2);
	STATIC_CHECK(Bousk::NbBits<4>::Value == 3);
	STATIC_CHECK(Bousk::NbBits<5>::Value == 3);
	STATIC_CHECK(Bousk::NbBits<8>::Value == 4);
	STATIC_CHECK(Bousk::NbBits<12>::Value == 4);
	STATIC_CHECK(Bousk::NbBits<16>::Value == 5);
	STATIC_CHECK(Bousk::NbBits<127>::Value == 7);
	STATIC_CHECK(Bousk::NbBits<255>::Value == 8);
	STATIC_CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint16>::max()>::Value == 16);
	STATIC_CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint16>::max() / 2>::Value == 15);
	STATIC_CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint16>::max() / 2 + 1>::Value == 16);
	STATIC_CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint32>::max()>::Value == 32);
	STATIC_CHECK(Bousk::NbBits<std::numeric_limits<Bousk::uint64>::max()>::Value == 64);

	{
		Bousk::Float<Bousk::float32, 0, 1, 2> f;
		f = 0.15f;
		CHECK_FLOATS_EQUAL(Bousk::float32, static_cast<Bousk::float32>(f), 0.15f, 2);
	}
	{
		Bousk::Float<Bousk::float64, 0, 1, 2, 2> f(0.15);
		// Error : 0.15 is not a valid value on [0,1] with a step of 0.2
		//CHECK_FLOATS_EQUAL(Bousk::float64, static_cast<Bousk::float64>(f), 0.15f);
		f = 0.16;
		CHECK_FLOATS_EQUAL(Bousk::float64, static_cast<Bousk::float64>(f), 0.16, 2);
	}
	{
		Bousk::Float<Bousk::float32, -1, 1, 2> f(-0.14f);
		CHECK_FLOATS_EQUAL(Bousk::float32, static_cast<Bousk::float32>(f), -0.14f, 2);
	}
	{
		Bousk::Float<Bousk::float64, -42, 76, 1> f(-38.3);
		CHECK_FLOATS_EQUAL(Bousk::float64, f.get(), -38.3, 1);
		f = 75.9;
		CHECK_FLOATS_EQUAL(Bousk::float64, f.get(), 75.9, 1);
		f = -42;
		CHECK_FLOATS_EQUAL(Bousk::float64, f.get(), -42, 1);
		f = 76;
		CHECK_FLOATS_EQUAL(Bousk::float64, f.get(), 76, 1);
		f = 0;
		CHECK_FLOATS_EQUAL(Bousk::float64, f.get(), 0, 1);
		f = -0.1;
		CHECK_FLOATS_EQUAL(Bousk::float64, f.get(), -0.1, 1);
		f = 0.1;
		CHECK_FLOATS_EQUAL(Bousk::float64, f.get(), 0.1, 1);
	}
	{
		Bousk::Float<Bousk::float32, -15, 15, 1, 3> f(-12.f);
		CHECK_FLOATS_EQUAL(Bousk::float32, f.get(), -12.f, 1);
		f = -9.3f;
		CHECK_FLOATS_EQUAL(Bousk::float32, f.get(), -9.3f, 1);
		f = 0;
		CHECK_FLOATS_EQUAL(Bousk::float32, f.get(), 0, 1);
		f = 12.6f;
		CHECK_FLOATS_EQUAL(Bousk::float32, f.get(), 12.6f, 1);
		f = 14.1f;
		CHECK_FLOATS_EQUAL(Bousk::float32, f.get(), 14.1f, 1);
	}
}