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
			class ConnectionHandler
			{
			public:
				ConnectionHandler() = default;
				bool connect(SOCKET sckt, const Address& address);
				std::unique_ptr<Messages::Connection> poll();
				const Address& connectedAddress() const { return mAddress; }

			private:
				pollfd mFd{ 0 };
				Address mAddress;
			};
			bool ConnectionHandler::connect(SOCKET sckt, const Address& address)
			{
				assert(sckt != INVALID_SOCKET);
				assert(address.isValid());
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
				int res = ::poll(&mFd, 1, 0);
				if (res < 0)
					return std::make_unique<Messages::Connection>(mAddress, mFd.fd, Messages::Connection::Result::Failed);
				else if (res > 0)
				{
					if (mFd.revents & POLLOUT)
					{
						return std::make_unique<Messages::Connection>(mAddress, mFd.fd, Messages::Connection::Result::Success);
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
				inline char* missingDataStartBuffer() { return reinterpret_cast<char*>(mBuffer.data() + mReceived); }
				inline int missingDataLength() const { return static_cast<int>(mBuffer.size() - mReceived); }
				void startHeaderReception();
				void startDataReception();
				void startReception(unsigned int expectedDataLength, State newState);

			private:
				std::vector<unsigned char> mBuffer;
				unsigned int mReceived{ 0 };
				SOCKET mSckt{ INVALID_SOCKET };
				Address mAddress;
				State mState{ State::Header };
			};
			void ReceptionHandler::init(SOCKET sckt, const Address& addr)
			{
				assert(sckt != INVALID_SOCKET);
				mSckt = sckt;
				mAddress = addr;
				startHeaderReception();
			}
			void ReceptionHandler::startHeaderReception()
			{
				startReception(HeaderSize, State::Header);
			}
			void ReceptionHandler::startDataReception()
			{
				assert(mBuffer.size() == sizeof(HeaderType));
				HeaderType networkExpectedDataLength;
				memcpy(&networkExpectedDataLength, mBuffer.data(), sizeof(networkExpectedDataLength));
				const auto expectedDataLength = ntohs(networkExpectedDataLength);
				startReception(expectedDataLength, State::Data);
			}
			void ReceptionHandler::startReception(unsigned int expectedDataLength, State newState)
			{
				mReceived = 0;
				mBuffer.clear();
				mBuffer.resize(expectedDataLength, 0);
				mState = newState;
			}
			std::unique_ptr<Messages::Base> ReceptionHandler::recv()
			{
				assert(mSckt != INVALID_SOCKET);
				int ret = ::recv(mSckt, missingDataStartBuffer(), missingDataLength(), 0);
				if (ret > 0)
				{
					mReceived += ret;
					if (mReceived == mBuffer.size())
					{
						if (mState == State::Data)
						{
							std::unique_ptr<Messages::Base> msg = std::make_unique<Messages::UserData>(mAddress, static_cast<uint64>(mSckt), std::move(mBuffer));
							startHeaderReception();
							return msg;
						}
						else
						{
							startDataReception();
							//!< if any data are already available they will then be returned
							return recv();
						}
					}
					return nullptr;
				}
				else if (ret == 0)
				{
					//!< connection ended properly
					return std::make_unique<Messages::Disconnection>(mAddress, static_cast<uint64>(mSckt), Messages::Disconnection::Reason::Disconnected);
				}
				else // ret < 0
				{
					//!< error handling
					int error = Errors::Get();
					if (error == Errors::WOULDBLOCK || error == Errors::AGAIN)
					{
						return nullptr;
					}
					else
					{
						return std::make_unique<Messages::Disconnection>(mAddress, static_cast<uint64>(mSckt), Messages::Disconnection::Reason::Lost);
					}
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
				void init(SOCKET sckt);
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
			void SendingHandler::init(SOCKET sckt)
			{
				mSocket = sckt;
				if (mState == State::Header || mState == State::Data)
				{
					mSendingBuffer.clear();
				}
				mState = State::Idle;
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
			}
			Client& Client::operator=(Client&& src) noexcept
			{
				std::swap(mConnectionHandler, src.mConnectionHandler);
				std::swap(mSendingHandler, src.mSendingHandler);
				std::swap(mReceivingHandler, src.mReceivingHandler);
				std::swap(mAddress, src.mAddress);
				std::swap(mSocket, src.mSocket);
				std::swap(mState, src.mState);
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
				onConnected(addr);
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
					auto msg = mConnectionHandler->poll();
					if (msg)
					{
						if (msg->result == Messages::Connection::Result::Success)
						{
							onConnected(mConnectionHandler->connectedAddress());
						}
						else
						{
							disconnect();
						}
					}
					return msg;
				} break;
				case State::Connected:
				{
					mSendingHandler->update();
					auto msg = mReceivingHandler->recv();
					if (msg)
					{
						if (msg->is<Messages::Disconnection>())
						{
							disconnect();
						}
					}
					return msg;
				} break;
				case State::Disconnected:
				{
				} break;
				}
				return nullptr;
			}
			void Client::onConnected(const Address& addr)
			{
				mAddress = addr;
				mSendingHandler->init(mSocket);
				mReceivingHandler->init(mSocket, mAddress);
				mState = State::Connected;
			}
		}
	}
}
