#include "Sockets.hpp"

#include <iostream>
#include <string>

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

	for (;;)
	{
		sockaddr_in from = { 0 };
		socklen_t addrlen = sizeof(from);
		SOCKET newClient = accept(server, (SOCKADDR*)(&from), &addrlen);
		if (newClient != INVALID_SOCKET)
		{
			std::string clientAddress = Sockets::GetAddress(from);
			std::cout << "Connexion de " << clientAddress.c_str() << ":" << ntohs(from.sin_port) << std::endl;
		}
		else
			break;
	}
	Sockets::CloseSocket(server);
	Sockets::Release();
	return 0;
}