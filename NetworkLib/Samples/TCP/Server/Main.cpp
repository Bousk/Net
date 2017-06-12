#include "Sockets.hpp"
#include "TCP/Server.hpp"
#include "Messages.hpp"
#include "Errors.hpp"

#include <iostream>

int main()
{
	if (!Network::Start())
	{
		std::cout << "Erreur initialisation WinSock : " << Network::Errors::Get();
		return -1;
	}
	
	unsigned short port;
	std::cout << "Port ? ";
	std::cin >> port;

	Network::TCP::Server server;
	if (!server.start(port))
	{
		std::cout << "Erreur initialisation serveur : " << Network::Errors::Get();
		return -2;
	}

	while(1)
	{
		server.update();
		while (auto msg = server.poll())
		{
			if (msg->is<Network::Messages::Connection>())
			{
				std::cout << "Connexion de [" << Network::GetAddress(msg->from) << ":" << Network::GetPort(msg->from) << "]" << std::endl;
			}
			else if (msg->is<Network::Messages::Disconnection>())
			{
				std::cout << "Deconnexion de [" << Network::GetAddress(msg->from) << ":" << Network::GetPort(msg->from) << "]" << std::endl;
			}
			else if (msg->is<Network::Messages::UserData>())
			{
				auto userdata = msg->as<Network::Messages::UserData>();
				server.sendToAll(userdata->data.data(), static_cast<unsigned int>(userdata->data.size()));
			}
		}
	}
	server.stop();
	Network::Release();
	return 0;
}