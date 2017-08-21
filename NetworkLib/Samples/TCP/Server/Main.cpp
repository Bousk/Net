#include "Sockets.hpp"
#include "TCP/Server.hpp"
#include "Messages.hpp"
#include "Errors.hpp"

#include <iostream>

int main()
{
	if (!Bousk::Network::Start())
	{
		std::cout << "Erreur initialisation WinSock : " << Bousk::Network::Errors::Get();
		return -1;
	}
	
	unsigned short port;
	std::cout << "Port ? ";
	std::cin >> port;

	Bousk::Network::TCP::Server server;
	if (!server.start(port))
	{
		std::cout << "Erreur initialisation serveur : " << Bousk::Network::Errors::Get();
		return -2;
	}

	while(1)
	{
		server.update();
		while (auto msg = server.poll())
		{
			if (msg->is<Bousk::Network::Messages::Connection>())
			{
				std::cout << "Connexion de [" << Bousk::Network::GetAddress(msg->from) << ":" << Bousk::Network::GetPort(msg->from) << "]" << std::endl;
			}
			else if (msg->is<Bousk::Network::Messages::Disconnection>())
			{
				std::cout << "Deconnexion de [" << Bousk::Network::GetAddress(msg->from) << ":" << Bousk::Network::GetPort(msg->from) << "]" << std::endl;
			}
			else if (msg->is<Bousk::Network::Messages::UserData>())
			{
				auto userdata = msg->as<Bousk::Network::Messages::UserData>();
				server.sendToAll(userdata->data.data(), static_cast<unsigned int>(userdata->data.size()));
			}
		}
	}
	server.stop();
	Bousk::Network::Release();
	return 0;
}