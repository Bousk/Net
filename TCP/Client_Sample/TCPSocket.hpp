#ifndef BOUSK_DVP_SOCKET_COURS_TCPSOCKET_HPP
#define BOUSK_DVP_SOCKET_COURS_TCPSOCKET_HPP

#pragma once

#include "Sockets.hpp"

#include <vector>
#include <string>

class TCPSocket
{
	public:
		TCPSocket();
		~TCPSocket();

		bool Connect(const std::string& ipaddress, unsigned short port);
		int Send(const unsigned char* data, unsigned int len);
		bool Receive(std::vector<unsigned char>& buffer);

	private:
		SOCKET mSocket;
};


#endif // BOUSK_DVP_SOCKET_COURS_TCPSOCKET_HPP