#include "Sockets.hpp"
#include "TCP/Server.hpp"
#include "Messages.hpp"
#include "Errors.hpp"

#include <iostream>

int main()
{
	if (!Bousk::Network::Start())
	{
		std::cout << "Network lib initialisation error : " << Bousk::Network::Errors::Get();
		return -1;
	}
	
	unsigned short port;
	std::cout << "Port ? ";
	std::cin >> port;

	Bousk::Network::TCP::Server server;
	if (!server.start(port))
	{
		std::cout << "Server initialisation error : " << Bousk::Network::Errors::Get();
		return -2;
	}

	while(1)
	{
		server.update();
		while (auto msg = server.poll())
		{
			if (msg->is<Bousk::Network::Messages::Connection>())
			{
				std::cout << "[" << msg->emitter().toString() << "] Connected" << std::endl;
			}
			else if (msg->is<Bousk::Network::Messages::Disconnection>())
			{
				std::cout << "[" << msg->emitter().toString() << "] Disconnected" << std::endl;
			}
			else if (msg->is<Bousk::Network::Messages::UserData>())
			{
				std::cout << "Data from [" << msg->emitter().toString() << "]" << std::endl;
				auto userdata = msg->as<Bousk::Network::Messages::UserData>();
				server.sendToAll(userdata->data.data(), static_cast<unsigned int>(userdata->data.size()));
			}
		}
	}
	server.stop();
	Bousk::Network::Release();
	return 0;
}