#include "UDP/AckHandler.hpp"
#include "Utils.hpp"
#include <algorithm>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			void AckHandler::update(const uint16 newAck, uint64 previousAcks, const bool trackLoss /*= false*/)
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
					const uint16 diff = Utils::SequenceDiff(newAck, mLastAck);
					const uint16 gap = diff - 1;
					//!< Bits to shift from mask, check for loss
					const uint8 bitsToShift = static_cast<uint8>(std::min(diff, static_cast<uint16>(64)));
					if (trackLoss)
					{
						for (uint8 i = 0; i < bitsToShift; ++i)
						{
							const uint8 packetDiffWithLastAck = 64 - i;
							const uint8 bitInPreviousMask = packetDiffWithLastAck - 1;
							if (!Utils::HasBit(mPreviousAcks, bitInPreviousMask))
							{
								//!< This packet hasn't been acked and is now out of range : lost
								const uint16 packetid = mLastAck - packetDiffWithLastAck;
								mLoss.push_back(packetid);
							}
						}
					}
					//!< Remove bits from the mask to the left : remove oldest packets
					if (bitsToShift >= 64)
					{
						//!< If we're removing all of them, shifting by 64 does nothing on Windows when compiling x64, so let's clearly erase them !
						mPreviousAcks = 0;
					}
					else
					{
						mPreviousAcks <<= bitsToShift;
					}
					if (gap >= 64)
					{
						//!< Whole new mask
						mPreviousAcks = mNewAcks = 0;
						//!< Catchup last packet in actual mask and notify loss for every missing one
						if (trackLoss)
						{
							for (uint16 p = 64; p < gap; ++p)
							{
								const uint16 packetid = mLastAck + (p - 64) + 1;
								mLoss.push_back(packetid);
							}
						}
					}
					else
					{
						//!< Mark previous last ack as acked in the mask of previous acks
						Utils::SetBit(mPreviousAcks, static_cast<uint8>(gap));
					}
					mLastAck = newAck;
					mLastAckIsNew = true;
					mNewAcks = (mPreviousAcks & previousAcks) ^ previousAcks;
					mPreviousAcks |= previousAcks;
				}
				else
				{
					//!< This is an old ack, if it's not too old it may contain interesting information on acks
					const uint16 diff = Utils::SequenceDiff(mLastAck, newAck);
					if (diff <= 64)
					{
						//!< Align previous mask to our sequence
						if (diff == 64)
						{
							//!< Special case as shifting by 64 does nothing on Win64
							previousAcks = 0;
						}
						else
						{
							previousAcks <<= diff;
						}
						//!< Set ack bit in the shifted mask
						const uint8 ackBitInMask = static_cast<uint8>(diff - 1);
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
			bool AckHandler::isAcked(const uint16 ack) const
			{
				if (ack == mLastAck)
					return true;
				if (Utils::IsSequenceNewer(ack, mLastAck))
					return false;
				const auto diff = Utils::SequenceDiff(mLastAck, ack);
				if (diff > 64)
					return false;
				const uint8 bitPosition = static_cast<uint8>(diff - 1);
				return Utils::HasBit(mPreviousAcks, bitPosition);
			}
			bool AckHandler::isNewlyAcked(const uint16 ack) const
			{
				if (ack == mLastAck)
					return mLastAckIsNew;
				if (Utils::IsSequenceNewer(ack, mLastAck))
					return false;
				const auto diff = Utils::SequenceDiff(mLastAck, ack);
				if (diff > 64)
					return false;
				const uint8 bitPosition = static_cast<uint8>(diff - 1);
				return Utils::HasBit(mNewAcks, bitPosition);
			}
			std::vector<uint16> AckHandler::getNewAcks() const
			{
				std::vector<uint16> newAcks;
				newAcks.reserve(65);
				for (uint8 i = 64; i != 0; --i)
				{
					const uint8 bitToCheck = i - 1;
					if (Utils::HasBit(mNewAcks, bitToCheck))
					{
						const uint16 id = mLastAck - i;
						newAcks.push_back(id);
					}
				}
				if (mLastAckIsNew)
					newAcks.push_back(mLastAck);
				return newAcks;
			}
		}
	}
}