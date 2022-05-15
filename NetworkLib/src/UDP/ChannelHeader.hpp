#pragma once

#include <Types.hpp>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			struct ChannelHeader
			{
				static constexpr uint8 Size = sizeof(uint32) + sizeof(uint16);

				uint32 channelIndex;
				uint16 datasize;
			};
		}
	}
}