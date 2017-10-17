#include "Utils.hpp"
#include <numeric>

namespace Bousk
{
	namespace Utils
	{
		std::chrono::milliseconds Now()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		}
		bool IsSequenceNewer(uint32_t sNew, uint32_t sLast)
		{
			if (sNew == sLast)
				return false;
			return (sNew > sLast && sNew - sLast <= std::numeric_limits<uint32_t>::max() / 2)
				|| (sNew < sLast && sLast - sNew > std::numeric_limits<uint32_t>::max() / 2); // overflow handling
		}
		uint32_t SequenceDiff(uint32_t sNew, uint32_t sLast)
		{
			if (sNew == sLast)
				return 0;
			if (sNew > sLast && sNew - sLast <= std::numeric_limits<uint32_t>::max() / 2)
				return sNew - sLast;
			//!< overflow handling
			return (std::numeric_limits<uint32_t>::max() - sLast) + sNew + 1; //!< +1 because if sLast == sMax && sNew == 0, the diff is 1
		}
	}
}