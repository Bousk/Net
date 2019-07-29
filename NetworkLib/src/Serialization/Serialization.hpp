#pragma once

#include <Types.hpp>

namespace Bousk
{
	namespace Serialization
	{
		class Serializer;
		class Deserializer;
		class Serializable
		{
		public:
			virtual ~Serializable() = default;

			virtual bool write(Serializer&) const = 0;
			virtual bool read(Deserializer&) = 0;
		};
	}
}