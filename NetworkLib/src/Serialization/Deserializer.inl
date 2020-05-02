#include <cassert>
#include <numeric>

namespace Bousk
{
	namespace Serialization
	{
		template<class CONTAINER>
		bool Deserializer::readContainer(CONTAINER& container)
		{
			uint8 nbElements;
			if (!read(nbElements))
				return false;
			container.reserve(nbElements);
			for (uint8 i = 0; i < nbElements; ++i)
			{
				typename CONTAINER::value_type element;
				if (!read(element))
					return false;
				container.push_back(element);
			}
			return  true;
		}
	}
}
