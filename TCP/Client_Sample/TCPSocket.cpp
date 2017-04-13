#include "TCPSocket.hpp"

#include <stdexcept>
#include <sstream>

TCPSocket::TCPSocket()
{
	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mSocket == INVALID_SOCKET)
	{
		std::ostringstream error;
		error << "Erreur initialisation socket [" << Sockets::GetError() << "]";
		throw std::runtime_error(error.str());
	}
}
TCPSocket::~TCPSocket()
{
	Sockets::CloseSocket(mSocket);
}
bool TCPSocket::Connect(const std::string& ipaddress, unsigned short port)
{
	sockaddr_in server;
	inet_pton(AF_INET, ipaddress.c_str(), &server.sin_addr.s_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	return connect(mSocket, (const sockaddr*)&server, sizeof(server)) == 0;
}
int TCPSocket::Send(const unsigned char* data, unsigned int len)
{
	unsigned short networkLen = htons(len);
	return send(mSocket, reinterpret_cast<const char*>(&networkLen), sizeof(networkLen), 0) == sizeof(networkLen)
		&& send(mSocket, reinterpret_cast<const char*>(data), len, 0) == len;
}
bool TCPSocket::Receive(std::vector<unsigned char>& buffer)
{
	unsigned short expectedSize;
	int pending = recv(mSocket, reinterpret_cast<char*>(&expectedSize), sizeof(expectedSize), 0);
	if (pending <= 0 || pending != sizeof(unsigned short))
	{
		//!< Erreur
		return false;
	}

	expectedSize = ntohs(expectedSize);
	buffer.resize(expectedSize);
	int receivedSize = 0;
	do {
		int ret = recv(mSocket, reinterpret_cast<char*>(&buffer[receivedSize]), (expectedSize - receivedSize) * sizeof(unsigned char), 0);
		if (ret <= 0)
		{
			//!< Erreur
			buffer.clear();
			return false;
		}
		else
		{
			receivedSize += ret;
		}
	} while (receivedSize < expectedSize);
	return true;
}