#ifndef SOCKET_H
#define SOCKET_H

#pragma once

#if defined(_WIN32)
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

	bool Create(SocketProtocol _protocol, SocketBlockingType _blockingType);
	bool Bind(int port = 0);

	bool Listen(int backlog);
	bool AcceptConnection(Socket* outSocket);

	bool Connect(IPV4Addr addr);

	bool SetBlocking(SocketBlockingType bt);

	bool SendData(const void* buffer, int buffLength, int* bytesSent);
	bool ReceiveData(void* buffer, int buffLength, int* bytesReceived, IPV4Addr* outAddr);

	bool Destroy();
};

bool StartUpSocketSystem();
bool ShutdownSocketSystem();

#endif

