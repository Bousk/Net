#include <cassert>
#include <numeric>

namespace Bousk
{
	namespace Serialization
	{
		template<class CONTAINER>
		bool Serializer::writeContainer(const CONTAINER& container)
		{
			assert(container.size() <= std::numeric_limits<uint8>::max());
			mBuffer.reserve(mBuffer.size() + container.size() + 1);
			if (!write(static_cast<uint8>(container.size())))
				return false;
			for (auto&& data : container)
			{
				if (!write(data))
					return false;
			}
			return true;
		}
	}
}