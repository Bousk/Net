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
		{
			fd_set set;
			timeval timeout = { 0 };
			FD_ZERO(&set);
			FD_SET(server, &set);
			int selectReady = select(static_cast<int>(server) + 1, &set, nullptr, nullptr, &timeout);
			if (selectReady == -1)
			{
				std::cout << "Erreur select pour accept : " << Sockets::GetError() << std::endl;
				break;
			}
			if (selectReady > 0)
			{
				sockaddr_in from = { 0 };
				socklen_t addrlen = sizeof(from);
				SOCKET newClientSocket = accept(server, (SOCKADDR*)(&from), &addrlen);
				if (newClientSocket != INVALID_SOCKET)
				{
					Client newClient;
					newClient.sckt = newClientSocket;
					newClient.addr = from;
					const std::string clientAddress = Sockets::GetAddress(from);
					const unsigned short clientPort = ntohs(from.sin_port);
					std::cout << "Connexion de " << clientAddress.c_str() << ":" << clientPort << std::endl;
					clients.push_back(newClient);
				}
			}
		}
		if (!clients.empty())
		{
			fd_set setReads;
			fd_set setWrite;
			fd_set setErrors;
			FD_ZERO(&setReads);
			FD_ZERO(&setWrite);
			FD_ZERO(&setErrors);
			int highestFd = 0;
			timeval timeout = { 0 };
			for (auto& client : clients)
			{
				FD_SET(static_cast<int>(client.sckt), &setReads);
				FD_SET(static_cast<int>(client.sckt), &setWrite);
				FD_SET(static_cast<int>(client.sckt), &setErrors);
				if (client.sckt > highestFd)
					highestFd = static_cast<int>(client.sckt);
			}
			int selectResult = select(highestFd + 1, &setReads, &setWrite, &setErrors, &timeout);
			if (selectResult == -1)
			{
				std::cout << "Erreur select pour clients : " << Sockets::GetError() << std::endl;
				break;
			}
			else if (selectResult > 0)
			{
				auto itClient = clients.begin();
				while (itClient != clients.end())
				{
					const std::string clientAddress = Sockets::GetAddress(itClient->addr);
					const unsigned short clientPort = ntohs(itClient->addr.sin_port);

					bool hasError = false;
					if (FD_ISSET(itClient->sckt, &setErrors))
					{
						socklen_t err;
						int errsize = sizeof(err);
						if (getsockopt(itClient->sckt, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &errsize) != 0)
						{
							std::cout << "Impossible de determiner l'erreur : " << Sockets::GetError() << std::endl;
							continue;
						}
						std::cout << "Erreur : " << err << std::endl;
						hasError = true;
					}
					else if (FD_ISSET(itClient->sckt, &setReads))
					{
						char buffer[200] = { 0 };
						int ret = recv(itClient->sckt, buffer, 199, 0);
						if (ret == 0)
						{
							std::cout << "Connexion terminee" << std::endl;
							hasError = true;
						}
						else if(ret == SOCKET_ERROR)
						{
							std::cout << "Erreur reception : " << Sockets::GetError() << std::endl;
							hasError = true;
						}
						else
						{
							std::cout << "[" << clientAddress << ":" << clientPort << "]" << buffer << std::endl;
							if (FD_ISSET(itClient->sckt, &setWrite))
							{
								ret = send(itClient->sckt, buffer, ret, 0);
								if (ret == 0 || ret == SOCKET_ERROR)
								{
									std::cout << "Erreur envoi" << Sockets::GetError() << std::endl;
									hasError = true;
								}
							}
						}
					}
					if (hasError)
					{
						//!< Déconnecté
						std::cout << "Deconnexion de [" << clientAddress << ":" << clientPort << "]" << std::endl;
						itClient = clients.erase(itClient);
					}
					else
					{
						++itClient;
					}
				}
			}
		}
	}
	Sockets::CloseSocket(server);
	Sockets::Release();
	return 0;
}