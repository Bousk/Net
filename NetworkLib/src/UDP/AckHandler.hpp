#pragma once

#include <Types.hpp>
#include <vector>
#include <limits>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			class AckHandler
			{
				friend class AckHandler_Test;
			public:
				AckHandler() = default;
				AckHandler(const AckHandler&) = default;
				AckHandler& operator=(const AckHandler&) = default;
				AckHandler(AckHandler&&) = default;
				AckHandler& operator=(AckHandler&&) = default;
				~AckHandler() = default;

				void update(uint16 newAck, uint64 previousAcks, bool trackLoss = false);
				bool isAcked(uint16 ack) const;
				bool isNewlyAcked(uint16 ack) const;

				uint16 lastAck() const { return mLastAck; }
				uint64 previousAcksMask() const { return mPreviousAcks; }
				std::vector<uint16> getNewAcks() const;
				std::vector<uint16>&& loss() { return std::move(mLoss); }

			private:
				uint64 mPreviousAcks{ std::numeric_limits<uint64>::max() };
				uint64 mNewAcks{ 0 };
				std::vector<uint16> mLoss;
				uint16 mLastAck{ std::numeric_limits<uint16>::max() };
				bool mLastAckIsNew{ false };
			};
		}
	}
}
