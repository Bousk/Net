#include "Sockets.hpp"
#include "TCP/Client.hpp"
#include "Messages.hpp"
#include "Errors.hpp"

#include <iostream>

int main()
{
	if ( !Bousk::Network::Start() )
	{
		std::cout << "Error starting sockets : " << Bousk::Network::Errors::Get() << std::endl;
		return -1;
	}

	Bousk::Network::TCP::Client client;
	int port;
	std::cout << "Port du serveur ? ";
	std::cin >> port;
	if (!client.connect("127.0.0.1", port))
	{
		std::cout << "Impossible de se connecter au serveur [127.0.0.1:" << port << "] : " << Bousk::Network::Errors::Get() << std::endl;
	}
	else
	{
		while (1)
		{
			while (auto msg = client.poll())
			{
				if (msg->is<Bousk::Network::Messages::Connection>())
				{
					auto connection = msg->as<Bousk::Network::Messages::Connection>();
					if (connection->result == Bousk::Network::Messages::Connection::Result::Success)
					{
						std::cin.ignore();
						std::cout << "Connecte!" << std::endl;
						std::cout << "Entrez une phrase >";
						std::string phrase;
						std::getline(std::cin, phrase);
						if (!client.send(reinterpret_cast<const unsigned char*>(phrase.c_str()), static_cast<unsigned int>(phrase.length())))
						{
							std::cout << "Erreur envoi : " << Bousk::Network::Errors::Get() << std::endl;
							break;
						}
					}
					else
					{
						std::cout << "Connexion echoue : " << static_cast<int>(connection->result) << std::endl;
						break;
					}
				}
				else if (msg->is<Bousk::Network::Messages::UserData>())
				{
					auto userdata = msg->as<Bousk::Network::Messages::UserData>();
					std::string reply(reinterpret_cast<const char*>(userdata->data.data()), userdata->data.size());
					std::cout << "Reponse du serveur : " << reply << std::endl;
					std::cout << ">";
					std::string phrase;
					std::getline(std::cin, phrase);
					if (!client.send(reinterpret_cast<const unsigned char*>(phrase.c_str()), static_cast<unsigned int>(phrase.length())))
					{
						std::cout << "Erreur envoi : " << Bousk::Network::Errors::Get() << std::endl;
						break;
					}
				}
				else if (msg->is<Bousk::Network::Messages::Disconnection>())
				{
					auto disconnection = msg->as<Bousk::Network::Messages::Disconnection>();
					std::cout << "Deconnecte : " << static_cast<int>(disconnection->reason) << std::endl;
					break;
				}
			}
		}
	}
	Bousk::Network::Release();
	return 0;
}
