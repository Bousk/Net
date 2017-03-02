#include "Sockets.hpp"
#include "Errors.hpp"

#include <iostream>
#include <string>
#include <vector>

struct Client {
	SOCKET sckt;
	sockaddr_in addr;
};

int main()
{
	if (!Sockets::Start())
	{
		std::cout << "Erreur initialisation WinSock : " << Sockets::GetError();
		return -1;
	}

	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == INVALID_SOCKET)
	{
		std::cout << "Erreur initialisation socket : " << Sockets::GetError();
		return -2;
	}

	if (!Sockets::SetNonBlocking(server))
	{
		std::cout << "Erreur settings non-bloquant : " << Sockets::GetError();
		return -3;
	}

	unsigned short port;
	std::cout << "Port ? ";
	std::cin >> port;
	sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	int res = bind(server, (sockaddr*)&addr, sizeof(addr));
	if (res != 0)
	{
		std::cout << "Erreur bind : " << Sockets::GetError();
		return -3;
	}

	res = listen(server, SOMAXCONN);
	if (res != 0)
	{
		std::cout << "Erreur listen : " << Sockets::GetError();
		return -4;
	}

	std::cout << "Server demarre sur le port " << port << std::endl;

	std::vector<Client> clients;
	for (;;)
	{
		sockaddr_in from = { 0 };
		socklen_t addrlen = sizeof(from);
		SOCKET newClientSocket = accept(server, (SOCKADDR*)(&from), &addrlen);
		if (newClientSocket != INVALID_SOCKET)
		{
			if (!Sockets::SetNonBlocking(newClientSocket))
			{
				std::cout << "Erreur settings nouveau socket non-bloquant : " << Sockets::GetError() << std::endl;
				Sockets::CloseSocket(newClientSocket);
				continue;
			}
			Client newClient;
			newClient.sckt = newClientSocket;
			newClient.addr = from;
			const std::string clientAddress = Sockets::GetAddress(from);
			const unsigned short clientPort = ntohs(from.sin_port);
			std::cout << "Connexion de " << clientAddress.c_str() << ":" << clientPort << std::endl;
			clients.push_back(newClient);
		}
		{
			auto itClient = clients.begin();
			while ( itClient != clients.end() )
			{
				const std::string clientAddress = Sockets::GetAddress(from);
				const unsigned short clientPort = ntohs(from.sin_port);
				char buffer[200] = { 0 };
				int ret = recv(itClient->sckt, buffer, 199, 0);
				if (ret == 0)
				{
					//!< Déconnecté
					std::cout << "Deconnexion de [" << clientAddress << ":" << clientPort << "]" << std::endl;
					itClient = clients.erase(itClient);
					continue;
				}
				if (ret == SOCKET_ERROR)
				{
					int error = Sockets::GetError();
					if (error != static_cast<int>(Sockets::Errors::WOULDBLOCK))
					{
						std::cout << "Deconnexion de [" << clientAddress << ":" << clientPort << "]" << std::endl;
						itClient = clients.erase(itClient);
						continue;
					}
					//!< il n'y avait juste rien à recevoir, on passe au suivant
					continue;
				}
				std::cout << "[" << clientAddress << ":" << clientPort << "]" << buffer << std::endl;
				ret = send(itClient->sckt, buffer, ret, 0);
				if (ret == 0 || ret == SOCKET_ERROR)
					break;
			}
		}
	}
	Sockets::CloseSocket(server);
	Sockets::Release();
	return 0;
}