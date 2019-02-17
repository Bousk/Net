#include <numeric>
#include <cassert>

namespace Bousk
{
	namespace Utils
	{
		std::chrono::milliseconds Now()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		}
		bool IsSequenceNewer(const uint16_t sNew, const uint16_t sLast)
		{
			if (sNew == sLast)
				return false;
			return (sNew > sLast && sNew - sLast <= std::numeric_limits<uint16_t>::max() / 2)
				|| (sNew < sLast && sLast - sNew > std::numeric_limits<uint16_t>::max() / 2); //!< overflow handling
		}
		uint16_t SequenceDiff(const uint16_t sNew, const uint16_t sLast)
		{
			if (sNew == sLast)
				return 0;
			//!< Please order the parameters or diff can't proceed
			assert(IsSequenceNewer(sNew, sLast));
			if (sNew > sLast && sNew - sLast <= std::numeric_limits<uint16_t>::max() / 2)
				return sNew - sLast;
			//!< overflow handling
			return (std::numeric_limits<uint16_t>::max() - sLast) + sNew + 1; //!< +1 because if sLast == sMax && sNew == 0, the diff is 1
		}
		void SetBit(uint64_t& bitfield, const uint8_t n)
		{
			assert(n < 64);
			bitfield |= (Bit<uint64_t>::Right << n);
		}
		void UnsetBit(uint64_t& bitfield, const uint8_t n)
		{
			assert(n < 64);
			bitfield &= (~(Bit<uint64_t>::Right << n));
		}
		bool HasBit(uint64_t bitfield, const uint8_t n)
		{
			assert(n < 64);
			return (bitfield & (Bit<uint64_t>::Right << n)) != 0;
		}
	}
}