#pragma once

#include <cstdint>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			struct ChannelHeader
			{
				static constexpr size_t Size = sizeof(uint32_t) + sizeof(uint32_t);

				uint32_t canalId;
				uint32_t datasize;
			};
		}
	}
}