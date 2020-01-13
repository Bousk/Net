#include "UDP/UDPClient.hpp"
#include "UDP/DistantClient.hpp"
#include "Address.hpp"
#include "Messages.hpp"
#include "Errors.hpp"

#include <algorithm>
#include <cstring>

namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			void Client::SetTimeout(std::chrono::milliseconds timeout)
			{
				DistantClient::SetTimeout(timeout);
			}
			std::chrono::milliseconds Client::GetTimeout()
			{
				return DistantClient::GetTimeout();
			}

			Client::Client()
			{}
			Client::~Client()
			{}

			bool Client::init(const uint16 port)
			{
				assert(!mRegisteredChannels.empty()); // Initializing without any channel doesn't make sense..

				release();
				mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
				if (mSocket == INVALID_SOCKET)
					return false;

				const Address addr = Address::Any(Address::Type::IPv4, port);
				if (!addr.bind(mSocket))
					return false;

				if (!SetNonBlocking(mSocket))
					return false;

				mClientIdsGenerator = 0;
				return true;
			}
			void Client::release()
			{
				if (mSocket != INVALID_SOCKET)
					CloseSocket(mSocket);
				mSocket = INVALID_SOCKET;
			}
			void Client::connect(const Address& addr)
			{
				OperationsLock lock(mOperationsLock);
				mPendingOperations.push_back(Operation::Connect(addr));
			}
			void Client::disconnect(const Address& addr)
			{
				OperationsLock lock(mOperationsLock);
				mPendingOperations.push_back(Operation::Disconnect(addr));
			}
			void Client::sendTo(const Address& target, std::vector<uint8>&& data, const uint32 channelIndex)
			{
				OperationsLock lock(mOperationsLock);
				mPendingOperations.push_back(Operation::SendTo(target, std::move(data), channelIndex));
			}
			void Client::processSend()
			{
				// Process pending operations
				std::vector<Operation> operations;
				{
					OperationsLock lock(mOperationsLock);
					operations.swap(mPendingOperations);
				}
				for (Operation& op : operations)
				{
					switch (op.mType)
					{
						case Operation::Type::Connect:
						{
							if (auto client = getClient(op.mTarget, true))
								client->connect();
						} break;
						case Operation::Type::SendTo:
						{
							if (auto client = getClient(op.mTarget, true))
								client->send(std::move(op.mData), op.mChannel);
						} break;
						case Operation::Type::Disconnect:
						{
							if (auto client = getClient(op.mTarget))
								client->disconnect();
						} break;
					}
				}

				// Do send data to clients
				for (auto& client : mClients)
					client->processSend();

				// Remove disconnected clients
				mClients.erase(
					std::remove_if(mClients.begin(), mClients.end(), [](const std::unique_ptr<DistantClient>& client) { return client->isDisconnected(); })
					, mClients.end()
				);
			}
			void Client::receive()
			{
				for (;;)
				{
					Datagram datagram;
					Address from;
					int ret = from.recvFrom(mSocket, reinterpret_cast<uint8*>(&datagram), Datagram::BufferMaxSize);
					if (ret > 0)
					{
						const size_t receivedSize = static_cast<size_t>(ret);
						if (receivedSize >= Datagram::HeaderSize)
						{
							datagram.datasize = receivedSize - Datagram::HeaderSize;
							if (auto client = getClient(from, true))
								client->onDatagramReceived(std::move(datagram));
						}
						else
						{
							//!< Something is wrong, unexpected datagram
						}
					}
					else
					{
						if (ret < 0)
						{
							//!< Error handling
							const auto err = Errors::Get();
							if (err != Errors::WOULDBLOCK)
							{
								//!< Log that error
							}
						}
						return;
					}
				}
			}
			std::vector<std::unique_ptr<Messages::Base>> Client::poll()
			{
				MessagesLock lock(mMessagesLock);
				return std::move(mMessages);
			}

			DistantClient* Client::getClient(const Address& clientAddr, bool create /*= false*/)
			{
				auto itClient = std::find_if(mClients.begin(), mClients.end(), [&](const std::unique_ptr<DistantClient>& client) { return client->address() == clientAddr; });
				if (itClient != mClients.end())
					return itClient->get();
				else if (create)
				{
					mClients.emplace_back(std::make_unique<DistantClient>(*this, clientAddr, mClientIdsGenerator++));
					setupChannels(*(mClients.back()));
					return mClients.back().get();
				}
				else
					return nullptr;
			}
			void Client::setupChannels(DistantClient& client)
			{
				for (auto& fct : mRegisteredChannels)
					fct(client);
			}
			void Client::onMessageReady(std::unique_ptr<Messages::Base>&& msg)
			{
				MessagesLock lock(mMessagesLock);
				mMessages.push_back(std::move(msg));
			}
		}
	}
}
