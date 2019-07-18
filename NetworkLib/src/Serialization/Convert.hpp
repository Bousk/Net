#pragma once

#include <Types.hpp>

namespace Bousk
{
	namespace Serialization
	{
		namespace Conversion
		{
			void ToNetwork(uint16 in, uint16& out);
			void ToNetwork(uint32 in, uint32& out);
			void ToNetwork(float32 in, uint32& out);

			void ToNetwork(uint16 in, uint8 out[2]);
			void ToNetwork(uint32 in, uint8 out[4]);

			void ToLocal(uint16 in, uint16& out);
			void ToLocal(uint32 in, uint32& out);
			void ToLocal(uint32 in, float32& out);

			void ToLocal(const uint8 in[2], uint16& out);
			void ToLocal(const uint8 in[4], uint32& out);
		}
	}
}