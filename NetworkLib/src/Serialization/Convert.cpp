#include <Serialization/Convert.hpp>

#ifdef _WIN32
	#define NOMINMAX
	#include <WinSock2.h>
#else
	#include <arpa/inet.h>
#endif

namespace Bousk
{
	namespace Serialization
	{
		namespace Conversion
		{
			void ToNetwork(const uint16 from, uint16& to)
			{
				to = htons(from);
			}
			void ToNetwork(const uint32 from, uint32& to)
			{
				to = htonl(from);
			}
			
			void ToNetwork(const uint16 in, uint8 out[2])
			{
				out[0] = static_cast<uint8>((in >> 8) & 0xFF);
				out[1] = static_cast<uint8>((in >> 0) & 0xFF);
			}
			void ToNetwork(const uint32 in, uint8 out[4])
			{
				out[0] = static_cast<uint8>((in >> 24) & 0xFF);
				out[1] = static_cast<uint8>((in >> 16) & 0xFF);
				out[2] = static_cast<uint8>((in >> 8) & 0xFF);
				out[3] = static_cast<uint8>((in >> 0) & 0xFF);
			}

			void ToLocal(const uint16 from, uint16& to)
			{
				to = ntohs(from);
			}
			void ToLocal(const uint32 from, uint32& to)
			{
				to = ntohl(from);
			}

			// Buffers from network are always Big-Endian (Network-Endian)
			void ToLocal(const uint8 in[2], uint16& out)
			{
				uint16 tmp;
				memcpy(&tmp, in, 2);
				ToLocal(tmp, out);
			}
			void ToLocal(const uint8 in[4], uint32& out)
			{
				uint32 tmp;
				memcpy(&tmp, in, 4);
				ToLocal(tmp, out);
			}

			union FloatConversionHelper
			{
				static_assert(sizeof(float32) == sizeof(uint32), "");
				float32 f;
				uint32 u;
			};
			void ToNetwork(float32 in, uint32& out)
			{
				FloatConversionHelper helper;
				helper.f = in;
				ToNetwork(helper.u, out);
			}
			void ToLocal(uint32 in, float32& out)
			{
				FloatConversionHelper helper;
				ToLocal(in, helper.u);
				out = helper.f;
			}
		}
	}
}