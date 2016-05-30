#ifdef _WIN32
#if _MSC_VER >= 1800
#include <WS2tcpip.h>
#else
#define inet_pton(FAMILY, IP, PTR_STRUCT_SOCKADDR) (*(PTR_STRUCT_SOCKADDR)) = inet_addr((IP))
typedef int socklen_t;
#endif
#include <WinSock2.h>
#ifdef _MSC_VER
#if _WIN32_WINNT == _WIN32_WINNT_WINBLUE
//!< Win8.1 & higher
#pragma comment(lib, "Ws2_32.lib")
#else
#pragma comment(lib, "wsock32.lib")
#endif
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in, IPPROTO_TCP
#include <arpa/inet.h> // hton*, ntoh*, inet_addr
#include <unistd.h>  // close
#include <fcntl.h>
#include <cerrno> // errno
#define SOCKET int
#define INVALID_SOCKET ((int)-1)
#endif

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

namespace
{
	bool Start()
	{
#ifdef _WIN32
		WSAData wsaData;
		return WSAStartup(MAKEWORD(2,2), &wsaData) == 0;
#else
		return true;
#endif
	}
	void End()
	{
#ifdef _WIN32
		WSACleanup();
#endif
	}
	int GetError()
	{
#ifdef _WIN32
		return WSAGetLastError();
#else
		return errno;
#endif
	}
	void CloseSocket(SOCKET s)
	{
#ifdef _WIN32
		closesocket(s);
#else
		close(s);
#endif
	}
	bool SetNonBlocking(SOCKET s)
	{
#ifdef _WIN32
		unsigned long iMode = 1;
		return ioctlsocket(s, FIONBIO, &iMode) == 0;
#else
		return fcntl(s, F_SETFL, O_NONBLOCK) == 0;
#endif
	}

	std::string ConvertAddr(const sockaddr_in& addr)
	{
#if defined(_WIN32) && _MSC_VER >= 1800
		char buff[32] = {0};
		InetNtop(addr.sin_family, (void*)&(addr.sin_addr), buff, 31);
		return std::string(buff, 32);
#else
		return inet_ntoa(addr.sin_addr);
#endif
	}

	namespace Errors
	{
		enum Type
		{
#ifdef _WIN32
			WOULDBLOCK = WSAEWOULDBLOCK,
#else
			WOULDBLOCK = EWOULDBLOCK,
#endif
		};
	}

#define BUFFER_MAX 1024
	bool Receive(SOCKET socket, std::string& _buffer)
	{
		char buffer[BUFFER_MAX + 1] = { 0 };
		int iLastRecievedBufferLen = 0;
		do {
			iLastRecievedBufferLen = recv(socket, buffer, BUFFER_MAX, 0);
			_buffer += buffer;
		} while (iLastRecievedBufferLen == BUFFER_MAX);
		if (iLastRecievedBufferLen > 0)
			return true;
		else
		{
			int error = GetError();
			if ( error == Errors::WOULDBLOCK )
				return true;
			return false;
		}
	}


	std::vector<std::string> Split(const std::string& str, const std::string& separator)
	{
		std::vector<std::string> parts;
		int start = 0;
		size_t end;
		while ( (end = str.find(separator, start)) != std::string::npos )
		{
			parts.push_back(str.substr(start, end - start));
			start = end + 1;
		}
		parts.push_back(str.substr(start));
		return parts;
	}
	std::string Merge(const std::vector<std::string>& parts, const std::string& aggregator)
	{
		if ( parts.empty() )
			return "";
		std::string result = parts[0];
		for ( size_t i = 1; i < parts.size(); ++i )
			result += aggregator + parts[i];
		return result;
	}
	std::string ShuffleSentence(const std::string& sentence)
	{
		std::vector<std::string> words = Split(sentence, " ");
		std::random_shuffle(words.begin(), words.end());
		return Merge(words, " ");
	}
}

struct Client {
	SOCKET socket;
	std::string ip;
	int port;
	Client(SOCKET _socket, const std::string& _ip, int _port)
		: socket(_socket)
		, ip(_ip)
		, port(_port)
	{}
	void Close() { CloseSocket(socket); socket = INVALID_SOCKET; }
};

bool Server(int port)
{
	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == INVALID_SOCKET)
		return false;
	sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY; // indique que toutes les adresses sont acceptées
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	int res = bind(server, (sockaddr*)&addr, sizeof(addr));
	if (res != 0)
		return false;
	res = listen(server, SOMAXCONN);
	if (res != 0)
		return false;
	SetNonBlocking(server);
	std::cout << "Server demarre sur le port " << port << std::endl;
	std::vector<Client> clients;
	for (;;)
	{
		for (;;)
		{
			SOCKET newClient;
			sockaddr addr = { 0 };
			socklen_t len = sizeof(addr);
			newClient = accept(server, &addr, &len);
			if (newClient == INVALID_SOCKET)
				break;
			sockaddr_in from;
			socklen_t addrlen = sizeof(from);
			if (getpeername(newClient, (sockaddr*)&from, &addrlen) != 0)
			{
				std::cout << "Nouveau client, Impossible de retrouver son IP : " << GetError() << " (deconnexion)" << std::endl;
				CloseSocket(newClient);
				continue;
			}
			Client client(newClient, ConvertAddr(from), ntohs(from.sin_port));
			std::cout << "Connexion de " << client.ip.c_str() << ":"<< client.port << std::endl;
			SetNonBlocking(newClient);
			clients.push_back(client);
		}
		std::vector<Client>::iterator client = clients.begin();
		while (client != clients.end())
		{
			std::string buffer;
			if (Receive(client->socket, buffer))
			{
				if ( !buffer.empty() )
				{
					std::cout << "Recu de [" << client->ip.c_str() << ":" << client->port << "] : " << buffer << std::endl;
					std::string reply = ShuffleSentence(buffer);
					std::cout << "Reponse a [" << client->ip.c_str() << ":" << client->port << "] > " << reply << std::endl;
					send(client->socket, reply.c_str(), reply.length(), 0);
				}
				++client;
			}
			else
			{
				client->Close();
				std::cout << "[" << client->ip.c_str() << ":" << client->port << "] " << " Deconnexion" << std::endl;
				client = clients.erase(client);
			}
		}
	}
}

int main()
{
	if ( !Start() )
	{
		std::cout << "Erreur initialisation sockets : " << GetError() << std::endl;
		return -1;
	}
	int port;
	std::cout << "Port du serveur > ";
	std::cin >> port;
	if (!Server(port))
	{
		std::cout << "Serveur termine avec l'erreur " << GetError() << std::endl;
	}
	End();
	return 0;
}
