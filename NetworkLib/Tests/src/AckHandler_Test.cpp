#pragma once

#include "AckHandler_Test.hpp"
#include "Tester.hpp"

#include <UDP/AckHandler.hpp>
#include <Utils.hpp>

void AckHandler_Test::Test()
{
	static constexpr uint64_t MASK_COMPLETE = std::numeric_limits<uint64_t>::max();
	static constexpr uint64_t MASK_FIRST_ACKED = Bousk::Utils::Bit<uint64_t>::Right;
	static constexpr uint64_t MASK_FIRST_MISSING = ~MASK_FIRST_ACKED;;
	static constexpr uint64_t MASK_LAST_ACKED = (MASK_FIRST_ACKED << 63);
	Bousk::Network::UDP::AckHandler ackhandler;
	CHECK(ackhandler.lastAck() == std::numeric_limits<uint16_t>::max());
	CHECK(ackhandler.previousAcksMask() == MASK_COMPLETE);
	CHECK(!ackhandler.isAcked(0));
	CHECK(!ackhandler.isNewlyAcked(0));
	CHECK(ackhandler.loss().empty());
	//!< Receive packet 0 with no gap
	ackhandler.update(0, MASK_COMPLETE, true);
	CHECK(ackhandler.lastAck() == 0);
	CHECK(ackhandler.previousAcksMask() == MASK_COMPLETE);
	CHECK(ackhandler.isAcked(0));
	CHECK(ackhandler.isNewlyAcked(0));
	CHECK(ackhandler.getNewAcks().size() == 1);
	CHECK(ackhandler.getNewAcks()[0] == 0);
	CHECK(ackhandler.loss().empty());
	//!< Receive packet 2, packet 1 is missing (63th bit of previous mask = 0)
	ackhandler.update(2, MASK_FIRST_MISSING, true);
	CHECK(ackhandler.lastAck() == 2);
	CHECK(ackhandler.previousAcksMask() == MASK_FIRST_MISSING);
	CHECK(ackhandler.isAcked(2));
	CHECK(ackhandler.isAcked(0));
	CHECK(ackhandler.isNewlyAcked(2));
	CHECK(!ackhandler.isNewlyAcked(0));
	CHECK(ackhandler.loss().empty());
	//!< Receive packet 1, no more gap
	ackhandler.update(1, MASK_COMPLETE, true);
	CHECK(ackhandler.lastAck() == 2);
	CHECK(ackhandler.previousAcksMask() == MASK_COMPLETE);
	CHECK(ackhandler.isAcked(1));
	CHECK(ackhandler.isAcked(2));
	CHECK(ackhandler.isAcked(0));
	CHECK(ackhandler.isNewlyAcked(1));
	CHECK(!ackhandler.isNewlyAcked(2));
	CHECK(!ackhandler.isNewlyAcked(0));
	CHECK(ackhandler.loss().empty());
	//!< Jump 64 packets ahead, all missed in between, but not lost yet
	ackhandler.update(66, 0, true);
	CHECK(ackhandler.lastAck() == 66);
	CHECK(ackhandler.isNewlyAcked(66));
	CHECK(ackhandler.previousAcksMask() == MASK_LAST_ACKED);
	CHECK(ackhandler.loss().empty());
	//!< Receive next one and everything is missing in between
	ackhandler.update(67, 0, true);
	CHECK(ackhandler.lastAck() == 67);
	CHECK(ackhandler.isNewlyAcked(67));
	CHECK(!ackhandler.isNewlyAcked(66));
	CHECK(ackhandler.previousAcksMask() == MASK_FIRST_ACKED);
	CHECK(ackhandler.loss().empty());
	//!< Receive next one and ack everything, 3 is now lost
	ackhandler.update(68, MASK_COMPLETE, true);
	CHECK(ackhandler.lastAck() == 68);
	CHECK(ackhandler.isNewlyAcked(68));
	CHECK(ackhandler.previousAcksMask() == MASK_COMPLETE);
	{
		auto loss = ackhandler.loss();
		CHECK(loss.size() == 1);
		CHECK(loss[0] == 3);
	}
	for (uint16_t i = 4; i < 66; ++i)
	{
		CHECK(ackhandler.isNewlyAcked(i));
	}
	//!< Receive too old ack
	ackhandler.update(0, 0, true);
	CHECK(ackhandler.lastAck() == 68);
	CHECK(!ackhandler.isNewlyAcked(68));
	CHECK(ackhandler.previousAcksMask() == MASK_COMPLETE);
	//!< Jump 65 ahead with all missing in between
	ackhandler.update(133, 0, true);
	CHECK(ackhandler.lastAck() == 133);
	CHECK(ackhandler.previousAcksMask() == 0);
	CHECK(ackhandler.loss().empty());
	//!< Ack all of them with previous ack
	ackhandler.update(132, MASK_COMPLETE, true);
	CHECK(ackhandler.lastAck() == 133);
	CHECK(ackhandler.previousAcksMask() == MASK_COMPLETE);
	CHECK(ackhandler.loss().empty());
	//!< Jump 100 with full mask, 134 to 134 + 100 - 65 = 169 should be lost
	ackhandler.update(234, 0, true);
	CHECK(ackhandler.lastAck() == 234);
	CHECK(ackhandler.previousAcksMask() == 0);
	{
		auto loss = ackhandler.loss();
		const auto firstLost = 134;
		const auto lastLost = 169;
		const auto totalLost = lastLost - firstLost + 1;
		CHECK(loss.size() == totalLost);
		for (auto i = 0; i < totalLost; ++i)
		{
			CHECK(loss[i] == firstLost + i);
		}
	}
	ackhandler.update(234, MASK_COMPLETE, true);
	ackhandler.update(236, MASK_COMPLETE, true);
	//!< Jump 65 ahead with all missing in between
	ackhandler.update(301, 0, true);
	CHECK(ackhandler.lastAck() == 301);
	CHECK(ackhandler.previousAcksMask() == 0);
	CHECK(ackhandler.loss().empty());
	CHECK(!ackhandler.isAcked(237));
	//!< Ack 237
	ackhandler.update(237, MASK_COMPLETE, true);
	CHECK(ackhandler.lastAck() == 301);
	CHECK(ackhandler.previousAcksMask() == MASK_LAST_ACKED);
	CHECK(ackhandler.loss().empty());
	CHECK(ackhandler.isAcked(237));
	CHECK(ackhandler.isNewlyAcked(237));
	//!< Ack all of them with same last ack
	ackhandler.update(301, MASK_COMPLETE, true);
	CHECK(ackhandler.lastAck() == 301);
	CHECK(ackhandler.previousAcksMask() == MASK_COMPLETE);
	CHECK(ackhandler.loss().empty());
	//!< Quick check mask => ids
	ackhandler.update(303, MASK_COMPLETE, true);
	auto newAcks = ackhandler.getNewAcks();
	CHECK(newAcks.size() == 2);
	CHECK(newAcks[0] == 302);
	CHECK(newAcks[1] == 303);
}