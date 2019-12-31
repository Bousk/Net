#include "Sockets.hpp"
#include "Errors.hpp"

#include <iostream>
#include <string>
#include <thread>

int main()
{
	if (!Bousk::Network::Start())
	{
		std::cout << "Network lib initialisation error : " << Bousk::Network::Errors::Get();
		return -1;
	}

	SOCKET myFirstUdpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (myFirstUdpSocket == SOCKET_ERROR)
	{
		std::cout << "Socket creation error : " << Bousk::Network::Errors::Get();
		return -2;
	}

	unsigned short port;
	std::cout << "Local port ? ";
	std::cin >> port;

	sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	if (bind(myFirstUdpSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
	{
		std::cout << "Socket bind error : " << Bousk::Network::Errors::Get();
		return -3;
	}

	unsigned short portDst;
	std::cout << "Target port ? ";
	std::cin >> portDst;
	sockaddr_in to = { 0 };
	inet_pton(AF_INET, "127.0.0.1", &to.sin_addr.s_addr);
	to.sin_family = AF_INET;
	to.sin_port = htons(portDst);

	std::cout << "Text to send (empty to quit)> ";
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
			std::cout << "Data send error : " << Bousk::Network::Errors::Get() << ". Closing program.";
			break;
		}
		char buff[1500] = { 0 };
		sockaddr_in from;
		socklen_t fromlen = sizeof(from);
		ret = recvfrom(myFirstUdpSocket, buff, 1499, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
		if (ret <= 0)
		{
			std::cout << "Data reception error : " << Bousk::Network::Errors::Get() << ". Closing program.";
			break;
		}
		std::cout << "Received : " << buff << " from " << Bousk::Network::GetAddress(from) << ":" << Bousk::Network::GetPort(from) << std::endl;
	}

	Bousk::Network::CloseSocket(myFirstUdpSocket);
	Bousk::Network::Release();
	return 0;
}