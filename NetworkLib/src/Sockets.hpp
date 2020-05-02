#pragma once

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #define WIN32_LEAN_AND_MEAN
    #if _WIN32_WINNT < 0x6000
        #undef _WIN32_WINNT
        #define _WIN32_WINNT _WIN32_WINNT_WINBLUE
    #endif
	#include <WinSock2.h>
	#ifdef __GNUC__
		#include <WS2tcpip.h>
        using nfds_t = unsigned long;
        inline int poll(pollfd fdarray[], nfds_t nfds, int timeout) { return WSAPoll(fdarray, nfds, timeout); }
	#elif defined(_MSC_VER)
		#if _MSC_VER >= 1800
            #include <WS2tcpip.h>
        #else
            #define inet_ntop(FAMILY, PTR_STRUCT_SOCKADDR, BUFFER, BUFFER_LENGTH) strncpy(BUFFER, inet_ntoa(*static_cast<struct in_addr*>((PTR_STRUCT_SOCKADDR))), BUFFER_LENGTH)
            #define inet_pton(FAMILY, IP, PTR_STRUCT_SOCKADDR) (*(PTR_STRUCT_SOCKADDR)) = inet_addr((IP))
            using socklen_t = int;
        #endif
		#if _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
			//!< Win8.1 & higher
			#pragma comment(lib, "Ws2_32.lib")
		#else
			#pragma comment(lib, "wsock32.lib")
		#endif
        #if _WIN32_WINNT >= 0x0600
            using nfds_t = unsigned long;
            inline int poll(pollfd fdarray[], nfds_t nfds, int timeout) { return WSAPoll(fdarray, nfds, timeout); }
		#else
			// TODO : fake poll using select internally
        #endif // _WIN32_WINNT
    #endif
#else
	#include <sys/socket.h>
	#include <netinet/in.h> // sockaddr_in, IPPROTO_TCP
	#include <arpa/inet.h> // hton*, ntoh*, inet_addr
	#include <unistd.h>  // close
	#include <cerrno> // errno
	#include <poll.h> // poll
	#include <fcntl.h>
	#define SOCKET int
	#define INVALID_SOCKET ((int)-1)
	#define SOCKET_ERROR (int(-1))
#endif

#include <string>

namespace Bousk
{
	namespace Network
	{
		bool Start();
		void Release();
		bool SetNonBlocking(SOCKET socket);
		bool SetReuseAddr(SOCKET socket);
		void CloseSocket(SOCKET socket);
		std::string GetAddress(const sockaddr_in& addr);
		unsigned short GetPort(const sockaddr_in& addr);

		inline std::string GetAddress(const sockaddr_storage& addr) { return GetAddress(reinterpret_cast<const sockaddr_in&>(addr)); }
		inline unsigned short GetPort(const sockaddr_storage& addr) { return GetPort(reinterpret_cast<const sockaddr_in&>(addr)); }
	}
}
