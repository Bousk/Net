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

	SOCKET myFirstUdpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (myFirstUdpSocket == SOCKET_ERROR)
	{
		std::cout << "Erreur création socket : " << Bousk::Network::Errors::Get();
		return -2;
	}

	unsigned short port;
	std::cout << "Port local ? ";
	std::cin >> port;

	sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	if (bind(myFirstUdpSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
	{
		std::cout << "Erreur bind socket : " << Bousk::Network::Errors::Get();
		return -3;
	}

	unsigned short portDst;
	std::cout << "Port du destinataire ? ";
	std::cin >> portDst;
	sockaddr_in to = { 0 };
	inet_pton(AF_INET, "127.0.0.1", &to.sin_addr.s_addr);
	to.sin_family = AF_INET;
	to.sin_port = htons(portDst);

	std::cout << "Entrez le texte a envoyer (vide pour quitter)> ";
	while (1)
	{
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
		char buff[1500] = { 0 };
		sockaddr_in from;
		socklen_t fromlen = sizeof(from);
		ret = recvfrom(myFirstUdpSocket, buff, 1499, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
		if (ret <= 0)
		{
			std::cout << "Erreur réception de données : " << Bousk::Network::Errors::Get() << ". Fermeture du programme.";
			break;
		}
		std::cout << "Recu : " << buff << " de " << Bousk::Network::GetAddress(from) << ":" << Bousk::Network::GetPort(from) << std::endl;
	}

	Bousk::Network::CloseSocket(myFirstUdpSocket);
	Bousk::Network::Release();
	return 0;
}