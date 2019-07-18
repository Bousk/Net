#include <cassert>
#include <numeric>

namespace Bousk
{
	namespace Serialization
	{
		template<class T>
		bool Serializer::write(const std::vector<T>& data)
		{
			assert(data.size() <= std::numeric_limits<uint8>::max());
			return writeArray(data.data(), static_cast<uint8>(data.size()));
		}

		template<class T>
		bool Serializer::writeArray(const T* data, uint8 nbElements)
		{
			mBuffer.reserve(mBuffer.size() + nbElements + 1);
			if (!write(nbElements))
				return false;
			for (uint8 i = 0; i < nbElements; ++i, ++data)
			{
				if (!write(*data))
					return false;
			}
			return true;
		}
	}
}