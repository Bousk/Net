#include <TCP/Client.hpp>

#include <Errors.hpp>
#include <Messages.hpp>

#include <cassert>
#include <limits>
#include <list>
#include <numeric>
#include <vector>

namespace Bousk
{
	namespace Network
	{
		namespace TCP
		{
			// Helper class to receive buffer of a given size.
			class SimpleReceiver
			{
			public:
				SimpleReceiver() = default;
				void init(SOCKET sckt)
				{
					mSckt = sckt;
				}
				// Returns false if the connection became invalid.
				// Returns true while the connection remains. Set buffer with the received data once the size received matches the expected one.
				bool recv(unsigned int expectedSize, std::vector<unsigned char>& buffer)
				{
					if (mBuffer.size() != expectedSize)
						mBuffer.resize(expectedSize);

					unsigned char* const recvBufferStart = mBuffer.data() + mReceived;
					const size_t missingBufferLength = mBuffer.size() - mReceived;
					mLastRcv = ::recv(mSckt, reinterpret_cast<char*>(recvBufferStart), static_cast<int>(missingBufferLength), 0);
					if (mLastRcv > 1)
					{
						mReceived += mLastRcv;
						if (mReceived == mBuffer.size())
						{
							buffer = std::move(mBuffer);
							mReceived = 0;
						}
						return true;
					}
					else if (mLastRcv < 0)
					{
						const int error = Errors::Get();
						if (error == Errors::WOULDBLOCK || error == Errors::AGAIN)
						{
							return true;
						}
					}
					return false;
				}
				int lastRecv() const { return mLastRcv; }

			private:
				std::vector<unsigned char> mBuffer;
				unsigned int mReceived{ 0 };
				int mLastRcv{ 0 };
				SOCKET mSckt{ INVALID_SOCKET };
			};

			class ConnectionHandler
			{
				enum class State {
					Connecting,
					WaitingConfirmation,
				};
			public:
				ConnectionHandler() = default;
				bool connect(SOCKET sckt, const Address& address);
				std::unique_ptr<Messages::Connection> poll();
				const Address& connectedAddress() const { return mAddress; }

			private:
				SimpleReceiver mReceiver;
				pollfd mFd{ 0 };
				Address mAddress;
				State mState{ State::Connecting };
			};
			bool ConnectionHandler::connect(SOCKET sckt, const Address& address)
			{
				assert(sckt != INVALID_SOCKET);
				assert(address.isValid());
				mReceiver.init(sckt);
				mAddress = address;
				mFd.fd = sckt;
				mFd.events = POLLOUT;
				if (!mAddress.connect(sckt))
				{
					int err = Errors::Get();
					if (err != Errors::INPROGRESS && err != Errors::WOULDBLOCK)
						return false;
				}
				return true;
			}
			std::unique_ptr<Messages::Connection> ConnectionHandler::poll()
			{
				if (mState == State::Connecting)
				{
					// Poll the socket for the async connect result
					const int res = ::poll(&mFd, 1, 0);
					if (res < 0)
						return std::make_unique<Messages::Connection>(mAddress, mFd.fd, Messages::Connection::Result::Failed);
					else if (res > 0)
					{
						if (mFd.revents & POLLOUT)
						{
							mState = State::WaitingConfirmation;
						}
						else if (mFd.revents & (POLLHUP | POLLNVAL))
						{
							return std::make_unique<Messages::Connection>(mAddress, mFd.fd, Messages::Connection::Result::Failed);
						}
						else if (mFd.revents & POLLERR)
						{
							return std::make_unique<Messages::Connection>(mAddress, mFd.fd, Messages::Connection::Result::Failed);
						}
					}
				}
				else if (mState == State::WaitingConfirmation)
				{
					// Receive 00 from the host to accept (dummy packet with no data)
					std::vector<unsigned char> buffer;
					if (!mReceiver.recv(HeaderSize, buffer))
					{
						return std::make_unique<Messages::Connection>(mAddress, mFd.fd, Messages::Connection::Result::Refused);
					}
					if (!buffer.empty())
					{
						HeaderType data;
						memcpy(&data, buffer.data(), sizeof(HeaderType));
						if (data == 0)
							return std::make_unique<Messages::Connection>(mAddress, mFd.fd, Messages::Connection::Result::Success);
						else
							return std::make_unique<Messages::Connection>(mAddress, mFd.fd, Messages::Connection::Result::Failed);
					}
				}
				//!< still in progress
				return nullptr;
			}
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			class ReceptionHandler
			{
				enum class State {
					Header,
					Data,
				};
			public:
				ReceptionHandler() = default;
				void init(SOCKET sckt, const Address& addr);
				std::unique_ptr<Messages::Base> recv();

			private:
				void startHeaderReception();
				void startDataReception(unsigned short expectedDataSize);

			private:
				SimpleReceiver mReceiver;
				SOCKET mSckt{ INVALID_SOCKET };
				Address mAddress;
				unsigned short mExpectedSize{ 0 };
				State mState{ State::Header };
			};
			void ReceptionHandler::init(SOCKET sckt, const Address& addr)
			{
				assert(sckt != INVALID_SOCKET);
				mSckt = sckt;
				mAddress = addr;
				mReceiver.init(mSckt);
				startHeaderReception();
			}
			void ReceptionHandler::startHeaderReception()
			{
				mExpectedSize = HeaderSize;
				mState = State::Header;
			}
			void ReceptionHandler::startDataReception(unsigned short expectedDataSize)
			{
				mExpectedSize = expectedDataSize;
				mState = State::Data;
			}
			std::unique_ptr<Messages::Base> ReceptionHandler::recv()
			{
				assert(mSckt != INVALID_SOCKET);
				std::vector<unsigned char> buffer;
				if (!mReceiver.recv(mExpectedSize, buffer))
				{
					if (mReceiver.lastRecv() == 0)
					{
						//!< connection ended properly
						return std::make_unique<Messages::Disconnection>(mAddress, static_cast<uint64>(mSckt), Messages::Disconnection::Reason::Disconnected);
					}
					else
					{
						return std::make_unique<Messages::Disconnection>(mAddress, static_cast<uint64>(mSckt), Messages::Disconnection::Reason::Lost);
					}
				}
				if (buffer.empty())
				{
					// Buffer not fully received yet
					return nullptr;
				}

				// Buffer is fully received and ready to process
				if (mState == State::Data)
				{
					std::unique_ptr<Messages::Base> msg = std::make_unique<Messages::UserData>(mAddress, static_cast<uint64>(mSckt), std::move(buffer));
					startHeaderReception();
					return msg;
				}
				else
				{
					HeaderType data;
					memcpy(&data, buffer.data(), sizeof(HeaderType));
					startDataReception(ntohs(data));
					//!< if any data are already available they will then be returned
					return recv();
				}
			}
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			class SendingHandler
			{
				enum class State {
					Idle,
					Header,
					Data,
				};
			public:
				SendingHandler() = default;
				void init(SOCKET sckt, bool sendConfirmation);
				bool send(const uint8* data, size_t datalen);
				void update();
				size_t queueSize() const;

			private:
				bool sendPendingBuffer();
				void prepareNextHeader();
				void prepareNextData();

			private:
				std::list<std::vector<uint8>> mQueueingBuffers;
				std::vector<uint8> mSendingBuffer;
				SOCKET mSocket{ INVALID_SOCKET };
				State mState{ State::Idle };
			};
			void SendingHandler::init(SOCKET sckt, bool sendConfirmation)
			{
				mSocket = sckt;
				if (mState == State::Header || mState == State::Data)
				{
					mSendingBuffer.clear();
				}
				mState = State::Idle;
				if (sendConfirmation)
				{
					// Send a dummy message just to notify the other end that the connection has been accepted in case no data have been queued yet.
					mQueueingBuffers.emplace_front();
				}
			}
			bool SendingHandler::send(const uint8* data, size_t datalen)
			{
				if (datalen > std::numeric_limits<HeaderType>::max())
					return false;
				mQueueingBuffers.emplace_back(data, data + datalen);
				return true;
			}
			void SendingHandler::update()
			{
				assert(mSocket != INVALID_SOCKET);
				if (mState == State::Idle && !mQueueingBuffers.empty())
				{
					prepareNextHeader();
				}
				while (mState != State::Idle && sendPendingBuffer())
				{
					if (mState == State::Header)
					{
						prepareNextData();
					}
					else
					{
						if (!mQueueingBuffers.empty())
						{
							prepareNextHeader();
						}
						else
						{
							mState = State::Idle;
						}
					}
				}
			}
			bool SendingHandler::sendPendingBuffer()
			{
				if (mSendingBuffer.empty())
					return true;

				//!< send remaining data from last send
				int sent = ::send(mSocket, reinterpret_cast<char*>(mSendingBuffer.data()), static_cast<int>(mSendingBuffer.size()), 0);
				if (sent > 0)
				{
					if (sent == mSendingBuffer.size())
					{
						//!< everything has been sent
						mSendingBuffer.clear();
						return true;
					}
					else
					{
						//!< partially sent
						memmove(mSendingBuffer.data() + sent, mSendingBuffer.data(), sent);
						mSendingBuffer.erase(mSendingBuffer.cbegin() + sent, mSendingBuffer.cend());
					}
				}
				return false;
			}
			void SendingHandler::prepareNextHeader()
			{
				assert(!mQueueingBuffers.empty());
				const auto header = static_cast<HeaderType>(mQueueingBuffers.front().size());
				const auto networkHeader = htons(header);
				mSendingBuffer.clear();
				mSendingBuffer.resize(HeaderSize);
				memcpy(mSendingBuffer.data(), &networkHeader, sizeof(HeaderType));
				mState = State::Header;
			}
			void SendingHandler::prepareNextData()
			{
				assert(!mQueueingBuffers.empty());
				mSendingBuffer.swap(mQueueingBuffers.front());
				mQueueingBuffers.pop_front();
				mState = State::Data;
			}
			size_t SendingHandler::queueSize() const
			{
				size_t s = std::accumulate(mQueueingBuffers.cbegin(), mQueueingBuffers.cend(), static_cast<size_t>(0), [](size_t n, const std::vector<unsigned char>& queuedItem) {
					return n + queuedItem.size() + HeaderSize;
				});
				if (mState == State::Data)
					s += mSendingBuffer.size();
				return s;
			}
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			Client::Client()
			{
				mConnectionHandler = std::make_unique<ConnectionHandler>();
				mSendingHandler = std::make_unique<SendingHandler>();
				mReceivingHandler = std::make_unique<ReceptionHandler>();
			}
			Client::Client(Client&& src) noexcept
			{
				std::swap(mConnectionHandler, src.mConnectionHandler);
				std::swap(mSendingHandler, src.mSendingHandler);
				std::swap(mReceivingHandler, src.mReceivingHandler);
				std::swap(mAddress, src.mAddress);
				std::swap(mSocket, src.mSocket);
				std::swap(mState, src.mState);
				std::swap(mIsServerClient, src.mIsServerClient);
			}
			Client& Client::operator=(Client&& src) noexcept
			{
				std::swap(mConnectionHandler, src.mConnectionHandler);
				std::swap(mSendingHandler, src.mSendingHandler);
				std::swap(mReceivingHandler, src.mReceivingHandler);
				std::swap(mAddress, src.mAddress);
				std::swap(mSocket, src.mSocket);
				std::swap(mState, src.mState);
				std::swap(mIsServerClient, src.mIsServerClient);
				return *this;
			}
			Client::~Client()
			{
				disconnect();
			}
			bool Client::init(SOCKET&& sckt, const Address& addr)
			{
				assert(sckt != INVALID_SOCKET);
				if (sckt == INVALID_SOCKET)
					return false;

				assert(mState == State::Disconnected);
				assert(mSocket == INVALID_SOCKET);
				if (mSocket != INVALID_SOCKET)
					disconnect();

				mSocket = sckt;
				if (!SetNonBlocking(mSocket))
				{
					disconnect();
					return false;
				}
				mIsServerClient = true;
				onWaitingConnectionToBeAccepted(addr);
				return true;
			}
			bool Client::connect(const Address& address)
			{
				assert(mState == State::Disconnected);
				assert(mSocket == INVALID_SOCKET);
				if (mSocket != INVALID_SOCKET)
					disconnect();
				mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (mSocket == INVALID_SOCKET)
				{
					return false;
				}
				else if (!SetNonBlocking(mSocket))
				{
					disconnect();
					return false;
				}
				if (mConnectionHandler->connect(mSocket, address))
				{
					mState = State::Connecting;
					return true;
				}
				return false;
			}
			void Client::accept()
			{
				assert(mState == State::WaitingConnectionToBeAccepted);
				assert(mSocket != INVALID_SOCKET);

				onConnectionConfirmed();
			}
			void Client::disconnect()
			{
				if (mSocket != INVALID_SOCKET)
				{
					CloseSocket(mSocket);
				}
				mSocket = INVALID_SOCKET;
				mState = State::Disconnected;
			}
			bool Client::send(const uint8* data, size_t len)
			{
				return mSendingHandler->send(data, len);
			}
			std::unique_ptr<Messages::Base> Client::poll()
			{
				switch (mState)
				{
				case State::Connecting:
				{
					if (auto msg = mConnectionHandler->poll())
					{
						if (msg->result == Messages::Connection::Result::Success)
						{
							mAddress = mConnectionHandler->connectedAddress();
							onConnectionConfirmed();
							return msg;
						}
						else
						{
							disconnect();
							return msg;
						}
					}
				} break;
				case State::WaitingConnectionToBeAccepted:
				{
					// This state is for pure server-clients only. Waiting for the server to accept the client.
					// This state is never used on a client.
					// On the server, this state is switched when calling accept.
					assert(mIsServerClient);
				} break;
				case State::Connected:
				{
					mSendingHandler->update();
					if (auto msg = mReceivingHandler->recv())
					{
						if (msg->is<Messages::Disconnection>())
						{
							disconnect();
						}
						return msg;
					}
				} break;
				case State::Disconnected:
				{
				} break;
				}
				return nullptr;
			}
			void Client::onWaitingConnectionToBeAccepted(const Address& addr)
			{
				mAddress = addr;
				mState = State::WaitingConnectionToBeAccepted;
			}
			void Client::onConnectionConfirmed()
			{
				mSendingHandler->init(mSocket, mIsServerClient);
				mReceivingHandler->init(mSocket, mAddress);
				mState = State::Connected;
			}
		}
	}
}
