#include "UDP/AckHandler.hpp"
#include "Utils.hpp"
#include <algorithm>

namespace Bousk
{
	namespace UDP
	{
		void AckHandler::update(uint16_t newAck, uint64_t previousAcks, bool trackLoss /*= false*/)
		{
			mLastAckIsNew = false;
			if (newAck == mLastAck)
			{
				//!< Last ack is the same but some previous may be new
				mNewAcks = (mPreviousAcks & previousAcks) ^ previousAcks;
				mPreviousAcks |= previousAcks;
			}
			else if (Utils::IsSequenceNewer(newAck, mLastAck))
			{
				//!< This is a more recent ack, check loss etc
				const auto diff = Utils::SequenceDiff(newAck, mLastAck);
				const auto gap = diff - 1;
				//!< Bits to shift from mask, check for loss
				const auto bitsToShift = std::min(diff, static_cast<uint16_t>(64));
				if (trackLoss)
				{
					for (uint32_t i = 0; i < bitsToShift; ++i)
					{
						const auto packetDiffWithLastAck = 64 - i;
						const auto bitInPreviousMask = packetDiffWithLastAck - 1;
						if (!Utils::HasBit(mPreviousAcks, bitInPreviousMask))
						{
							//!< This packet hasn't been acked and is now out of range : lost
							const uint16_t packetid = mLastAck - packetDiffWithLastAck;
							mLoss.push_back(packetid);
						}
					}
				}
				//!< Remove bits from the mask to the left : remove oldest packets
				mPreviousAcks <<= bitsToShift;
				if (gap >= 64)
				{
					//!< Whole new mask
					mPreviousAcks = mNewAcks = 0;
					//!< Catchup last packet in actual mask and notify loss for every missing one
					if (trackLoss)
					{
						for (uint32_t p = 64; p < static_cast<uint32_t>(gap); ++p)
						{
							const uint16_t packetid = mLastAck + (p - 64) + 1;
							mLoss.push_back(packetid);
						}
					}
				}
				else
				{
					//!< Mark previous last ack as acked in the mask of previous acks
					Utils::SetBit(mPreviousAcks, gap);
				}
				mLastAck = newAck;
				mLastAckIsNew = true;
				mNewAcks = (mPreviousAcks & previousAcks) ^ previousAcks;
				mPreviousAcks |= previousAcks;
			}
			else
			{
				//!< This is an old ack, if it's not too old it may contain interesting information on acks
				const auto diff = Utils::SequenceDiff(mLastAck, newAck);
				if (diff <= 64)
				{
					//!< Align previous mask to our sequence
					previousAcks <<= diff;
					//!< Set ack bit in the shifted mask
					const auto ackBitInMask = diff - 1;
					Utils::SetBit(previousAcks, ackBitInMask);
					mNewAcks = (mPreviousAcks & previousAcks) ^ previousAcks;
					mPreviousAcks |= previousAcks;
				}
				else
				{
					//!< Way too old, just skip it
				}
			}
		}
		bool AckHandler::isAcked(uint16_t ack) const
		{
			if (ack == mLastAck)
				return true;
			if (Utils::IsSequenceNewer(ack, mLastAck))
				return false;
			const auto diff = Utils::SequenceDiff(mLastAck, ack);
			if (diff > 64)
				return false;
			const uint8_t bitPosition = static_cast<uint8_t>(diff - 1);
			return Utils::HasBit(mPreviousAcks, bitPosition);
		}
		bool AckHandler::isNewlyAcked(uint16_t ack) const
		{
			if (ack == mLastAck)
				return mLastAckIsNew;
			if (Utils::IsSequenceNewer(ack, mLastAck))
				return false;
			const auto diff = Utils::SequenceDiff(mLastAck, ack);
			if (diff > 64)
				return false;
			const uint8_t bitPosition = static_cast<uint8_t>(diff - 1);
			return Utils::HasBit(mNewAcks, bitPosition);
		}
		std::vector<uint16_t> AckHandler::getNewAcks() const
		{
			std::vector<uint16_t> newAcks;
			newAcks.reserve(65);
			for (uint8_t i = 64; i != 0; --i)
			{
				const uint8_t bitToCheck = i - 1;
				if (Utils::HasBit(mNewAcks, bitToCheck))
				{
					const uint16_t id = mLastAck - i;
					newAcks.push_back(id);
				}
			}
			if (mLastAckIsNew)
				newAcks.push_back(mLastAck);
			return newAcks;
		}
	}
}