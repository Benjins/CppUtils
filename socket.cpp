#include "socket.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )
#else
#include <errno.h>
#endif

#if defined(_WIN32)
typedef int socklen_t;
#endif

#if defined(_WIN32)
#define BNS_SOCKET_ERROR() printf("Socket error: %d\n", WSAGetLastError())
#else
#define BNS_SOCKET_ERROR() printf("Socket error: %s\n", strerror(errno))
#endif

IPV4Addr::IPV4Addr(int _addr, short _port){
	addr = _addr;
	port = _port;
}

IPV4Addr::IPV4Addr(unsigned char a, unsigned char b, unsigned char c, unsigned char d, short _port){
	addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
	port = htons(_port);
}

IPV4Addr::IPV4Addr(const char* hostName, int _port){
#if defined(_WIN32)
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* result;
	int error = getaddrinfo(hostName, NULL, &hints, &result);
	if (result != nullptr) {
		addr = ((sockaddr_in*)(result->ai_addr))->sin_addr.s_addr;
		port = htons(_port);
		freeaddrinfo(result);
	}
	else {
		printf("\nError resolving host: '%s'\n", hostName);
	}
#else
#endif
}

void IPV4Addr::WriteToString(char* buffer, int bufferSize){
	int hostAddr = ntohl(addr);
	short hostPort = ntohs(port);
	snprintf(buffer, bufferSize, "%d.%d.%d.%d:%d", (hostAddr >> 24), (hostAddr >> 16) & 0xFF, (hostAddr >> 8) & 0xFF, hostAddr & 0xFF, hostPort);
}

sockaddr_in IPV4Addr::ToSockAddr(){
	sockaddr_in ip = {};
	ip.sin_addr.s_addr = addr;
	ip.sin_port = port;
	ip.sin_family = AF_INET;

	return ip;
}

bool Socket::Create(SocketProtocol _protocol, SocketBlockingType _blockingType){
	protocol = _protocol;
	blockingType = _blockingType;
	
	if (protocol == SP_UDP){
		handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	else if (protocol == SP_TCP){
		handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else{
		//uuuuhhhh...
	}

	if (handle <= 0){
	}

#if defined(_WIN32)
	if (_blockingType == SBT_NonBlocking){
		u_long nonBlocking = 1;
		if (ioctlsocket( handle, FIONBIO, &nonBlocking ) != 0){
			printf( "failed to set non-blocking\n" );
			return false;
		}
	}
#else
	if (_blockingType == SBT_NonBlocking){
		int nonBlocking = 1;
		if (fcntl( handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1){
			printf( "failed to set non-blocking\n" );
			return false;
		}
	}
#endif

return true;
}

bool Socket::Bind(int _port /*= 0*/){
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short)_port);

	source.addr = htonl((127 << 24) | 1);

	int retVal = bind(handle, (const sockaddr*) &address, sizeof(sockaddr_in));

	if (retVal < 0 ){
		printf( "failed to bind socket\n" );
		return false;
	}

	if (_port == 0) {
		sockaddr boundAddr;

		socklen_t boundLen = sizeof(sockaddr);
		int val = getsockname(handle, &boundAddr, &boundLen);
		
		if (val != 0) {
#if defined(_WIN32)
	printf("\nWarning: could not determine port of socket (error %d).\n", WSAGetLastError());
#else
	printf("\nWarning: could not determine port of socket.\n");
#endif
			source.port = 0;
		}
		else {
			source.port = ((sockaddr_in*)(&boundAddr))->sin_port;
		}
	}
	else {
		source.port = htons(_port);
	}
	return true;
}

bool Socket::Connect(IPV4Addr addr){
	destination = addr;
	source = IPV4Addr(127, 0, 0, 1, source.port);

	sockaddr_in dstAddr = destination.ToSockAddr();
	int ret = connect(handle, (sockaddr*)(&dstAddr), sizeof(dstAddr));

	if (ret != 0) {
#if defined(_WIN32)
		int err = WSAGetLastError();
		printf("connect failed (err: %d)\n", err);
#else
		printf("connect failed\n");
#endif
		return false;
	}
	else {
		return true;
	}
}

bool Socket::SetBlocking(SocketBlockingType bt) {
#if defined(_WIN32)
	u_long iMode = (bt == SBT_NonBlocking ? 1 : 0);
	int rv = ioctlsocket(handle, FIONBIO, &iMode);

	return rv == 0;
#else
	int nonBlocking = (bt == SBT_NonBlocking ? 1 : 0);
	if (fcntl( handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1){
		printf( "failed to set non-blocking\n" );
		return false;
	}
	else{
		return true;
	}
#endif	
}

bool Socket::SendData(const void* buffer, int buffLength, int* bytesSent){
	char srcAddr[256] = {};
	char dstAddr[256] = {};

	source.WriteToString(srcAddr, sizeof(srcAddr));
	destination.WriteToString(dstAddr, sizeof(dstAddr));

	printf("Sending %d bytes from '%s' to '%s'\n", buffLength, srcAddr, dstAddr);

	sockaddr_in address = destination.ToSockAddr();
	
	int sentBytes = sendto(handle, (const char*)buffer, buffLength, 0, (sockaddr*)&address, sizeof(sockaddr_in));

	if (sentBytes != buffLength){
#if defined(_WIN32)
		int lastErr = WSAGetLastError();
		printf("failed to send packet (err: %d)\n", lastErr);
#else
		printf("failed to send packet.\n");
#endif
		return false;
	}

	*bytesSent = sentBytes;

	return true;
}

bool Socket::ReceiveData(void* buffer, int buffLength, int* bytesReceived, IPV4Addr* outAddr){
	sockaddr_in from = {};
	
	socklen_t fromLen = sizeof(from);
	int receivedBytes = recvfrom(handle, (char*)buffer, buffLength, 0, (sockaddr*)&from, &fromLen);
	
	if (receivedBytes == -1 && errno != EWOULDBLOCK){
		printf("Socket encountered error '%s'\n", strerror(errno));
	}

	IPV4Addr addr;
	addr.addr = from.sin_addr.s_addr;
	addr.port = from.sin_port;

	*outAddr = addr;

	*bytesReceived = receivedBytes;

	return receivedBytes > 0;
}

bool Socket::Listen(int backlog){
	int retVal = listen(handle, backlog);
	
	if (retVal == 0){
		return true;
	}
	else{
		BNS_SOCKET_ERROR();
		return false;
	}
}

bool Socket::AcceptConnection(Socket* outSocket){
	sockaddr_in from = {};
	socklen_t fromLen = sizeof(from);
	int rv = accept(handle, (sockaddr*)&from, &fromLen);
	
	if (rv < 0){
		return false;
	}
	else{
		*outSocket = *this;
		outSocket->handle = rv;
		
		outSocket->SetBlocking(blockingType);
		
		IPV4Addr addr;
		addr.addr = from.sin_addr.s_addr;
		addr.port = from.sin_port;
		
		outSocket->destination = addr;
		
		return true;
	}
}

bool Socket::Destroy(){
#if defined(_WIN32)
	// TODO: close for windows	
	closesocket(handle);
	return true;
#else
	close(handle);
	return true;
#endif
}

bool isSocketSystemInitialised = false;

bool StartUpSocketSystem(){
#if defined(_WIN32)
	WSADATA WsaData;
    return WSAStartup(MAKEWORD(2,2), &WsaData) == NO_ERROR;
#else
	return true;
#endif
}

bool ShutdownSocketSystem(){
	return true;
}

#if defined(SOCKET_TEST_MAIN)

int main(int argc, char** argv){
	
	if(StartUpSocketSystem()){
		printf("Failed to init socket system, exiting.\n");
		return -1;
	}

	Socket sock1;
	sock1.Create(SP_UDP, SBT_NonBlocking);
	sock1.Bind(12245);

	Socket sock2;
	sock2.Create(SP_UDP, SBT_NonBlocking);
	sock2.Bind(12243);

	sock1.Connect(sock2.source);

	const char* dataToSend = "Hello World";

	int bytesSent = 0;
	sock1.SendData(dataToSend, strlen(dataToSend)+1, &bytesSent);

	usleep(1000);

	char packet[256];
	int bytesReceived = 0;
	int timeout = 0;
	while (bytesReceived <= 0){
		IPV4Addr addr;
		sock2.ReceiveData(packet, sizeof(packet), &bytesReceived, &addr);
		usleep(1000);

		timeout++;
		if (timeout > 10){
			break;
		}
	}

	printf("Received '%s' from socket.\n", packet);

	if (strcmp(packet, dataToSend) != 0){
		return -1;
	}

	return 0;
}

#endif
