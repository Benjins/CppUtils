#ifndef SOCKET_H
#define SOCKET_H

#pragma once

#if defined(_WIN32)

// This is a tragic hack used in case windows.h is included beforehand
// Requires a #define _WINSOCKAPI_ before any windows.h is included.
// I'm sorry.
#if defined(_WINSOCKAPI_)
#undef _WINSOCKAPI_
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#endif

struct sockaddr_in;

struct IPV4Addr{
	int addr;
	short port;

	IPV4Addr() = default;

	IPV4Addr(int _addr, short _port);
	IPV4Addr(unsigned char a, unsigned char b, unsigned char c, unsigned char d, short _port);
	IPV4Addr(const char* hostName, int _port);

	bool operator==(const IPV4Addr& param){
		return addr == param.addr && port == param.port;
	}

	void WriteToString(char* buffer, int bufferSize);

	sockaddr_in ToSockAddr();
};

enum SocketBlockingType{
	SBT_Blocking,
	SBT_NonBlocking
};

enum SocketProtocol{
	SP_TCP,
	SP_UDP
};

struct Socket{
	int handle;
	SocketBlockingType blockingType;
	SocketProtocol protocol;

	IPV4Addr source;
	IPV4Addr destination;

	Socket(){
		handle = -1;
	}

	bool Create(SocketProtocol _protocol, SocketBlockingType _blockingType);
	bool Bind(int port = 0);

	bool Listen(int backlog);
	bool AcceptConnection(Socket* outSocket);

	bool Connect(IPV4Addr addr);

	bool SetBlocking(SocketBlockingType bt);

	bool SendData(const void* buffer, int buffLength, int* bytesSent);
	bool StreamDataTo(const void* buffer, int buffLength, int* bytesSent, IPV4Addr dst);
	bool ReceiveData(void* buffer, int buffLength, int* bytesReceived, IPV4Addr* outAddr);

	bool Destroy();
};

bool StartUpSocketSystem();
bool ShutdownSocketSystem();

#endif

