#pragma once

#include "DistantClient_Test.hpp"
#include "Tester.hpp"

#include <UDP/DistantClient.hpp>
#include <UDP/UDPClient.hpp>
#include <UDP/Packet.hpp>
#include <UDP/ChannelHeader.hpp>
#include <UDP/Protocols/UnreliableOrdered.hpp>
#include <Messages.hpp>
#include <Utils.hpp>

void DistantClient_Test::Test()
{
	static constexpr uint64_t MASK_COMPLETE = std::numeric_limits<uint64_t>::max();
	static constexpr uint64_t MASK_FIRST_ACKED = Bousk::Utils::Bit<uint64_t>::Right;
	static constexpr uint64_t MASK_FIRST_AND_SECOND_ACKED = (MASK_FIRST_ACKED << 1) | Bousk::Utils::Bit<uint64_t>::Right;
	static constexpr uint64_t MASK_FIRST_MISSING = ~MASK_FIRST_ACKED;
	static constexpr uint64_t MASK_LAST_ACKED = (MASK_FIRST_ACKED << 63);

	Bousk::Network::UDP::Client client; //!< We need a client to test DistantClient
	Bousk::Network::Address localAddress = Bousk::Network::Address::Loopback(Bousk::Network::Address::Type::IPv4, 8888); //!< Pretend we're a loopback distant client
	Bousk::Network::UDP::DistantClient distantClient(client, localAddress, 0);
	distantClient.registerChannel<Bousk::Network::UDP::Protocols::UnreliableOrdered>(std::nullopt);
	distantClient.onConnected();
	// Consume connection message
	{
		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 1);
		const auto& msg = polledMessages[0];
		CHECK(msg->is<Bousk::Network::Messages::Connection>());
	}

	CHECK(distantClient.mNextDatagramIdToSend == 0);
	CHECK(distantClient.mReceivedAcks.lastAck() == std::numeric_limits<uint16_t>::max());

	constexpr const char TestString[] = "Test data";
	constexpr size_t TestStringLength = sizeof(TestString);

	//!< Craft the datagram to check reception
	Bousk::Network::UDP::Datagram datagram;

	auto QueueDatagram = [&]() {
		distantClient.send(std::vector<uint8_t>(TestString, TestString + TestStringLength), 0);
		datagram.datasize = distantClient.mChannelsHandler.serialize(datagram.data.data(), Bousk::Network::UDP::Datagram::DataMaxSize, distantClient.mNextDatagramIdToSend, false);
		distantClient.fillDatagramHeader(datagram, Bousk::Network::UDP::Datagram::Type::ConnectedData);
	};

	QueueDatagram();
	CHECK(distantClient.mNextDatagramIdToSend == 1);
	CHECK(datagram.header.id == 0);
	CHECK(datagram.datasize == TestStringLength + Bousk::Network::UDP::Packet::HeaderSize + Bousk::Network::UDP::ChannelHeader::Size);

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
	QueueDatagram();
	datagram.header.id = htons(2);
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
	QueueDatagram();
	datagram.header.id = htons(1);
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
	QueueDatagram();
	datagram.header.id = htons(66);
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
	QueueDatagram();
	datagram.header.id = htons(67);
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
	QueueDatagram();
	datagram.header.id = htons(68);
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
	QueueDatagram();
	datagram.header.id = htons(3);
	{
		distantClient.onDatagramReceived(std::move(datagram));
		CHECK(distantClient.mReceivedAcks.lastAck() == 68);
		CHECK(!distantClient.mReceivedAcks.isNewlyAcked(68));
		CHECK(distantClient.mReceivedAcks.previousAcksMask() == MASK_FIRST_AND_SECOND_ACKED);

		auto polledMessages = client.poll();
		CHECK(polledMessages.size() == 0);
	}
}
