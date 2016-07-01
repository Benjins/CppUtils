#include "socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

IPV4Addr::IPV4Addr(int _addr, short _port){
	addr = _addr;
	port = _port;
}

IPV4Addr::IPV4Addr(unsigned char a, unsigned char b, unsigned char c, unsigned char d, short _port){
	addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
	port = htons(_port);
}

IPV4Addr::IPV4Addr(const char* hostName, int _port){

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

	return ip;
}

/**
int handle;
SocketBlockingType blockingType;
SocketProtocol protocol;
IPV4Addr source;
IPV4Addr destination;
*/

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
	}

	if (handle <= 0){
	}

	if (_blockingType == SBT_NonBlocking){
		int nonBlocking = 1;
		if (fcntl( handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1){
			printf( "failed to set non-blocking\n" );
			return false;
		}
	}
}

bool Socket::Bind(int _port /*= 0*/){
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short)_port);

	source.addr = htonl((127 << 24) | 1);
	source.port = htons(_port);
	bind(handle, (const sockaddr*) &address, sizeof(sockaddr_in));
	if (source.port < 0 ){
		printf( "failed to bind socket\n" );
		return false;
	}
}

bool Socket::Connect(IPV4Addr addr){
	destination = addr;
	source = IPV4Addr(127, 0, 0, 1, ntohs(source.port));
}

bool Socket::SendData(const void* buffer, int buffLength, int* bytesSent){
	char srcAddr[256] = {};
	char dstAddr[256] = {};

	source.WriteToString(srcAddr, sizeof(srcAddr));
	destination.WriteToString(dstAddr, sizeof(dstAddr));

	printf("Sending %d bytes from '%s' to '%s'\n", buffLength, srcAddr, dstAddr);

	sockaddr_in address = destination.ToSockAddr();
	int sentBytes =
		sendto(handle, (const char*)buffer, buffLength, 0, (sockaddr*)&address, sizeof(sockaddr_in));

	if (sentBytes != buffLength){
		printf("failed to send packet\n");
		return false;
	}

	*bytesSent = sentBytes;

	return true;
}

bool Socket::ReceiveData(void* buffer, int buffLength, int* bytesReceived, IPV4Addr* outAddr){
	sockaddr_in from;
	socklen_t fromLen = sizeof(from);

	int receivedBytes =
		recvfrom(handle, buffer, buffLength, 0, (sockaddr*)&from, &fromLen);

	IPV4Addr addr;
	addr.addr = from.sin_addr.s_addr;
	addr.port = from.sin_port;

	*outAddr = addr;

	*bytesReceived = receivedBytes;

	return receivedBytes > 0;
}

bool Socket::Destroy(){
	close(handle);
}

bool isSocketSystemInitialised = false;

bool StartUpSocketSystem(){
}

bool ShutdownSocketSystem(){
}

#if defined(SOCKET_TEST_MAIN)

int main(int argc, char** argv){

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
