#pragma once

#include <cstdint>
#include <vector>

namespace Bousk
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

			void update(uint16_t newAck, uint64_t previousAcks, bool trackLoss = false);
			bool isAcked(uint16_t ack) const;
			bool isNewlyAcked(uint16_t ack) const;

			uint16_t lastAck() const { return mLastAck; }
			uint64_t previousAcksMask() const { return mPreviousAcks; }
			std::vector<uint16_t> getNewAcks() const;
			std::vector<uint16_t>&& loss() { return std::move(mLoss); }

		private:
			uint64_t mPreviousAcks{ (uint64_t)-1 };
			uint64_t mNewAcks{ 0 };
			std::vector<uint16_t> mLoss;
			uint16_t mLastAck{ (uint16_t)-1 };
			bool mLastAckIsNew{ false };
		};
	}
}