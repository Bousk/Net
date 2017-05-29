#include "Server.hpp"

#include "Sockets.hpp"
#include "TCP/Client.hpp"
#include "Messages.hpp"

#include <map>
#include <list>

namespace Network
{
	namespace TCP
	{
		class ServerImpl
		{
			public:
				ServerImpl();
				~ServerImpl();

				bool start(unsigned short _port);
				void stop();
				void update();
				std::unique_ptr<Messages::Base> poll();

			private:
				std::map<SOCKET, Client> mClients;
				std::list<std::unique_ptr<Messages::Base>> mMessages;
		};
		////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////
		Server::Server();
		Server::~Server();

		bool Server::start(unsigned short _port) { return mImpl && mImpl->start(_port); }
		void Server::stop() { if (mImpl) mImpl->stop(); }
		void Server::update() { if (mImpl) mImpl->update(); }
		std::unique_ptr<Messages::Base> Server::poll() { return mImpl ? mImpl->poll() : nullptr; }
	}
}