#pragma once

#include "Tester.hpp"
#include "UDP/DistantClient.hpp"
#include "UDP/UDPClient.hpp"
#include "Messages.hpp"

#include <cstring>

class DistantClient_Test
{
public:
	static void Test();
};

void DistantClient_Test::Test()
{
	static constexpr uint64_t MASK_COMPLETE = std::numeric_limits<uint64_t>::max();
	static constexpr uint64_t MASK_FIRST_ACKED = Bousk::Utils::Bit<uint64_t>::Right;
	static constexpr uint64_t MASK_FIRST_AND_SECOND_ACKED = (MASK_FIRST_ACKED << 1) | Bousk::Utils::Bit<uint64_t>::Right;
	static constexpr uint64_t MASK_FIRST_MISSING = ~MASK_FIRST_ACKED;
	static constexpr uint64_t MASK_LAST_ACKED = (MASK_FIRST_ACKED << 63);

	Bousk::Network::UDP::Client client; //!< We need a client to test DistantClient
	sockaddr_in localAddress;
	localAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //!< Pretend we're a loopback distant client
	localAddress.sin_family = AF_INET;
	localAddress.sin_port = htons(8888);
	Bousk::Network::UDP::DistantClient distantClient(client, reinterpret_cast<const sockaddr_storage&>(localAddress));

	CHECK(distantClient.mNextDatagramIdToSend == 0);
	CHECK(distantClient.mReceivedAcks.lastAck() == std::numeric_limits<uint16_t>::max());

	constexpr const char TestString[] = "Test data";
	constexpr size_t TestStringLength = sizeof(TestString);

	distantClient.send(std::vector<uint8_t>(TestString, TestString + TestStringLength));
	//!< Craft the datagram to check reception
	Bousk::Network::UDP::Datagram datagram;
	distantClient.fillDatagram(datagram);
	CHECK(distantClient.mNextDatagramIdToSend == 1);

	auto QueueDatagram = [&]() {
		distantClient.send(std::vector<uint8_t>(TestString, TestString + TestStringLength));
		distantClient.mSendQueue.serialize(datagram.data.data(), Bousk::Network::UDP::Datagram::DataMaxSize);
	};

	CHECK(datagram.header.id == 0);
	CHECK(datagram.datasize == TestStringLength + Bousk::Network::UDP::Packet::HeaderSize);

	{
		distantClient.onDatagramReceived(std::move(datagram));
		CHECK(distantClient.mReceivedAcks.lastAck() == 0);
		CHECK(distantClient.mReceivedAcks.previousAcksMask() == MASK_COMPLETE);

		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 1);
		const auto& msg = polledMessages[0];
		CHECK(msg->is<Bousk::Network::Messages::UserData>());
		const auto dataMsg = msg->as<Bousk::Network::Messages::UserData>();
		CHECK(dataMsg->data.size() == TestStringLength);
		CHECK(memcmp(TestString, dataMsg->data.data(), TestStringLength) == 0);
	}

	//!< Datagram #0 received duplicate : will be ignored
	datagram.header.id = 0;
	{
		distantClient.onDatagramReceived(std::move(datagram));
		CHECK(distantClient.mReceivedAcks.lastAck() == 0);
		CHECK(distantClient.mReceivedAcks.previousAcksMask() == MASK_COMPLETE);

		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 0);
	}

	//!< Receive datagram #2, #1 is now missing
	datagram.header.id = htons(2);
	QueueDatagram();
	{
		distantClient.onDatagramReceived(std::move(datagram));
		CHECK(distantClient.mReceivedAcks.lastAck() == 2);
		CHECK(distantClient.mReceivedAcks.previousAcksMask() == MASK_FIRST_MISSING);

		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 1);
		const auto& msg = polledMessages[0];
		CHECK(msg->is<Bousk::Network::Messages::UserData>());
		const auto dataMsg = msg->as<Bousk::Network::Messages::UserData>();
		CHECK(dataMsg->data.size() == TestStringLength);
		CHECK(memcmp(TestString, dataMsg->data.data(), TestStringLength) == 0);
	}

	//!< Now receive datagram #1
	datagram.header.id = htons(1);
	QueueDatagram();
	{
		distantClient.onDatagramReceived(std::move(datagram));
		CHECK(distantClient.mReceivedAcks.lastAck() == 2);
		CHECK(distantClient.mReceivedAcks.isNewlyAcked(1));
		CHECK(!distantClient.mReceivedAcks.isNewlyAcked(2));
		CHECK(distantClient.mReceivedAcks.previousAcksMask() == MASK_COMPLETE);

		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 1);
		const auto& msg = polledMessages[0];
		CHECK(msg->is<Bousk::Network::Messages::UserData>());
		const auto dataMsg = msg->as<Bousk::Network::Messages::UserData>();
		CHECK(dataMsg->data.size() == TestStringLength);
		CHECK(memcmp(TestString, dataMsg->data.data(), TestStringLength) == 0);
	}

	//!< Jump 64 packets ahead, all missed in between
	datagram.header.id = htons(66);
	QueueDatagram();
	{
		distantClient.onDatagramReceived(std::move(datagram));
		CHECK(distantClient.mReceivedAcks.lastAck() == 66);
		CHECK(distantClient.mReceivedAcks.isNewlyAcked(66));
		CHECK(distantClient.mReceivedAcks.previousAcksMask() == MASK_LAST_ACKED);
		CHECK(distantClient.mReceivedAcks.loss().empty());

		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 1);
		const auto& msg = polledMessages[0];
		CHECK(msg->is<Bousk::Network::Messages::UserData>());
		const auto dataMsg = msg->as<Bousk::Network::Messages::UserData>();
		CHECK(dataMsg->data.size() == TestStringLength);
		CHECK(memcmp(TestString, dataMsg->data.data(), TestStringLength) == 0);
	}

	//!< Receive next one and everything is missing in between
	datagram.header.id = htons(67);
	QueueDatagram();
	{
		distantClient.onDatagramReceived(std::move(datagram));
		CHECK(distantClient.mReceivedAcks.lastAck() == 67);
		CHECK(distantClient.mReceivedAcks.isNewlyAcked(67));
		CHECK(distantClient.mReceivedAcks.previousAcksMask() == MASK_FIRST_ACKED);
		CHECK(distantClient.mReceivedAcks.loss().empty());

		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 1);
		const auto& msg = polledMessages[0];
		CHECK(msg->is<Bousk::Network::Messages::UserData>());
		const auto dataMsg = msg->as<Bousk::Network::Messages::UserData>();
		CHECK(dataMsg->data.size() == TestStringLength);
		CHECK(memcmp(TestString, dataMsg->data.data(), TestStringLength) == 0);
	}

	//!< Receive next one, #3 is now lost
	datagram.header.id = htons(68);
	QueueDatagram();
	{
		distantClient.onDatagramReceived(std::move(datagram));
		CHECK(distantClient.mReceivedAcks.lastAck() == 68);
		CHECK(distantClient.mReceivedAcks.isNewlyAcked(68));
		CHECK(distantClient.mReceivedAcks.previousAcksMask() == MASK_FIRST_AND_SECOND_ACKED);

		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 1);
		const auto& msg = polledMessages[0];
		CHECK(msg->is<Bousk::Network::Messages::UserData>());
		const auto dataMsg = msg->as<Bousk::Network::Messages::UserData>();
		CHECK(dataMsg->data.size() == TestStringLength);
		CHECK(memcmp(TestString, dataMsg->data.data(), TestStringLength) == 0);
	}

	//!< Receive datagram #3 : too old, ignored
	datagram.header.id = htons(3);
	QueueDatagram();
	{
		distantClient.onDatagramReceived(std::move(datagram));
		CHECK(distantClient.mReceivedAcks.lastAck() == 68);
		CHECK(!distantClient.mReceivedAcks.isNewlyAcked(68));
		CHECK(distantClient.mReceivedAcks.previousAcksMask() == MASK_FIRST_AND_SECOND_ACKED);

		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 0);
	}
}
