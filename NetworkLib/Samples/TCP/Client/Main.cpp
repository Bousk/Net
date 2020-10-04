#include "Sockets.hpp"
#include "TCP/Client.hpp"
#include "Messages.hpp"
#include "Errors.hpp"

#include <iostream>

int main()
{
	if ( !Bousk::Network::Start() )
	{
		std::cout << "Network lib initialisation error : " << Bousk::Network::Errors::Get();
		return -1;
	}

	Bousk::Network::TCP::Client client;
	int port;
	std::cout << "Server port ? ";
	std::cin >> port;
	if (!(port > 0 && port < std::numeric_limits<Bousk::uint16>::max()))
	{
		std::cout << "Invalid port" << std::endl;
		return -2;
	}
	Bousk::Network::Address serverAddress("127.0.0.1", static_cast<Bousk::uint16>(port));
	if (!client.connect(serverAddress))
	{
		std::cout << "Unable to connect to server [127.0.0.1:" << port << "] : " << Bousk::Network::Errors::Get() << std::endl;
	}
	else
	{
		for (bool run = true; run; )
		{
			while (auto msg = client.poll())
			{
				if (msg->is<Bousk::Network::Messages::Connection>())
				{
					auto connection = msg->as<Bousk::Network::Messages::Connection>();
					if (connection->result == Bousk::Network::Messages::Connection::Result::Success)
					{
						std::cin.ignore();
						std::cout << "Connected!" << std::endl;
						std::cout << "Enter a sentence >";
						std::string phrase;
						std::getline(std::cin, phrase);
						if (!client.send(reinterpret_cast<const unsigned char*>(phrase.c_str()), static_cast<unsigned int>(phrase.length())))
						{
							std::cout << "Send error : " << Bousk::Network::Errors::Get() << std::endl;
							run = false;
							break;
						}
					}
					else
					{
						std::cout << "Connection failed : " << static_cast<int>(connection->result) << std::endl;
						run = false;
						break;
					}
				}
				else if (msg->is<Bousk::Network::Messages::UserData>())
				{
					auto userdata = msg->as<Bousk::Network::Messages::UserData>();
					std::string reply(reinterpret_cast<const char*>(userdata->data.data()), userdata->data.size());
					std::cout << "From server : " << reply << std::endl;
					std::cout << ">";
					std::string phrase;
					std::getline(std::cin, phrase);
					if (!client.send(reinterpret_cast<const unsigned char*>(phrase.c_str()), static_cast<unsigned int>(phrase.length())))
					{
						std::cout << "Send error : " << Bousk::Network::Errors::Get() << std::endl;
						run = false;
						break;
					}
				}
				else if (msg->is<Bousk::Network::Messages::Disconnection>())
				{
					auto disconnection = msg->as<Bousk::Network::Messages::Disconnection>();
					std::cout << "Disconnected : " << static_cast<int>(disconnection->reason) << std::endl;
					run = false;
					break;
				}
			}
		}
	}
	Bousk::Network::Release();
	return 0;
}
