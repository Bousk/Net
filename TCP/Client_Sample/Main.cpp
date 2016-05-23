#include "Sockets.hpp"
#include "TCPSocket.hpp"

#include <iostream>

int main()
{
	if ( !Sockets::Start() )
	{
		std::cout << "Error starting sockets : " << Sockets::GetError() << std::endl;
		return -1;
	}
	TCPSocket client;
	int port;
	std::cout << "Port du serveur ? ";
	std::cin >> port;
	if (!client.Connect("127.0.0.1", port))
	{
		std::cout << "Impossible de se connecter au serveur [127.0.0.1:" << port << "] : " << Sockets::GetError() << std::endl;
	}
	else
	{
		std::cout << "Connecte" << std::endl;
	}
	Sockets::Release();
	return 0;
}
