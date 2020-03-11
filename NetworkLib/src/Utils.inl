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
		bool IsSequenceNewer(const uint16 sNew, const uint16 sLast)
		{
			if (sNew == sLast)
				return false;
			return (sNew > sLast && sNew - sLast <= std::numeric_limits<uint16>::max() / 2)
				|| (sNew < sLast && sLast - sNew > std::numeric_limits<uint16>::max() / 2); //!< overflow handling
		}
		uint16 SequenceDiff(const uint16 sNew, const uint16 sLast)
		{
			if (sNew == sLast)
				return 0;
			//!< Please order the parameters or diff can't proceed
			assert(IsSequenceNewer(sNew, sLast));
			if (sNew > sLast && sNew - sLast <= std::numeric_limits<uint16>::max() / 2)
				return sNew - sLast;
			//!< overflow handling
			return (std::numeric_limits<uint16>::max() - sLast) + sNew + 1; //!< +1 because if sLast == sMax && sNew == 0, the diff is 1
		}
		void SetBit(uint64& bitfield, const uint8 n)
		{
			assert(n < 64);
			bitfield |= (Bit<uint64>::Right << n);
		}
		void UnsetBit(uint64& bitfield, const uint8 n)
		{
			assert(n < 64);
			bitfield &= (~(Bit<uint64>::Right << n));
		}
		bool HasBit(uint64 bitfield, const uint8 n)
		{
			assert(n < 64);
			return (bitfield & (Bit<uint64>::Right << n)) != 0;
		}
	}
}