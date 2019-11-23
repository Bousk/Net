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
			void ToNetwork(uint64 in, uint64& out)
			{
				out = static_cast<uint8>((in >> 56) & 0xFF)
					| static_cast<uint8>((in >> 48) & 0xFF)
					| static_cast<uint8>((in >> 40) & 0xFF)
					| static_cast<uint8>((in >> 32) & 0xFF)
					| static_cast<uint8>((in >> 24) & 0xFF)
					| static_cast<uint8>((in >> 16) & 0xFF)
					| static_cast<uint8>((in >> 8) & 0xFF)
					| static_cast<uint8>((in >> 0) & 0xFF)
					;
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
			void ToNetwork(uint64 in, uint8 out[8])
			{
				out[0] = static_cast<uint8>((in >> 56) & 0xFF);
				out[1] = static_cast<uint8>((in >> 48) & 0xFF);
				out[2] = static_cast<uint8>((in >> 40) & 0xFF);
				out[3] = static_cast<uint8>((in >> 32) & 0xFF);
				out[4] = static_cast<uint8>((in >> 24) & 0xFF);
				out[5] = static_cast<uint8>((in >> 16) & 0xFF);
				out[6] = static_cast<uint8>((in >> 8) & 0xFF);
				out[7] = static_cast<uint8>((in >> 0) & 0xFF);
			}

			void ToLocal(const uint16 from, uint16& to)
			{
				to = ntohs(from);
			}
			void ToLocal(const uint32 from, uint32& to)
			{
				to = ntohl(from);
			}
			void ToLocal(uint64 in, uint64& out)
			{
				const uint8* inBuffer = reinterpret_cast<const uint8*>(&in);
				out = static_cast<uint64>(inBuffer[0]) << 56
					| static_cast<uint64>(inBuffer[1]) << 48
					| static_cast<uint64>(inBuffer[2]) << 40
					| static_cast<uint64>(inBuffer[3]) << 32
					| static_cast<uint64>(inBuffer[4]) << 24
					| static_cast<uint64>(inBuffer[5]) << 16
					| static_cast<uint64>(inBuffer[6]) << 8
					| static_cast<uint64>(inBuffer[7]) << 0
					;
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
			void ToLocal(const uint8 in[8], uint64& out)
			{
				uint64 tmp;
				memcpy(&tmp, in, 8);
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