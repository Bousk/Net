#include "Errors.hpp"

namespace Bousk
{
	namespace Network
	{
		namespace Errors
		{
			int Get()
			{
			#ifdef _WIN32
				return WSAGetLastError();
			#else
				return errno;
			#endif
			}
		}
	}
}