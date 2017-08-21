#include "Sockets.hpp"
#include "Errors.hpp"

#include <iostream>
#include <string>
#include <thread>

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

	SOCKET myFirstUdpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (myFirstUdpSocket == SOCKET_ERROR)
	{
		std::cout << "Erreur création socket : " << Bousk::Network::Errors::Get();
		return -2;
	}

	sockaddr_in to = { 0 };
	inet_pton(AF_INET, "127.0.0.1", &to.sin_addr.s_addr);
	to.sin_family = AF_INET;
	to.sin_port = htons(port);

	std::cout << "Entrez le texte a envoyer (vide pour quitter)> ";
	while (1)
	{
		std::string data;
		std::getline(std::cin, data);
		if (data.empty())
			break;
		int ret = sendto(myFirstUdpSocket, data.data(), static_cast<int>(data.length()), 0, reinterpret_cast<const sockaddr*>(&to), sizeof(to));
		if (ret <= 0)
		{
			std::cout << "Erreur envoi de données : " << Bousk::Network::Errors::Get() << ". Fermeture du programme.";
			break;
		}
	}

	Bousk::Network::CloseSocket(myFirstUdpSocket);
	Bousk::Network::Release();
	return 0;
}