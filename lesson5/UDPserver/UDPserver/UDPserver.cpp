// UDPserver.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WINSOCK2.H>   
#include <WS2tcpip.h>
#include <iostream>

#pragma comment(lib,"WS2_32.lib")

#define BUF_SIZE 1024

int main()
{
	WSADATA wsaData;
	SOCKET recvSocket;
	sockaddr_in recvAddr;
	char recvBuf[BUF_SIZE];
	sockaddr_in senderAddr;
	int senderAddrSize = sizeof(senderAddr);
	int res;

	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		printf("WSAStartup() failed with error %d\n", res);
		WSACleanup();
		return -1;
	}
	if ((recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("socket() failed with error %d \n", WSAGetLastError());
		return -1;
	}
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(10000);
	recvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	res = bind(recvSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr));
	if (res == SOCKET_ERROR) {
		printf("bind() failed with error %d \n", WSAGetLastError());
		return -1;
	}
	printf("Receiving datagrams...\n");

	char IPtemp[20];

	while (true)
	{
		ZeroMemory(recvBuf, BUF_SIZE);
		res = recvfrom(recvSocket, recvBuf, BUF_SIZE, 0, (SOCKADDR*)&senderAddr, &senderAddrSize);
		if (SOCKET_ERROR == res) {
			continue;
		}
		printf("Recv From Client [%s:%d] :%s\n", inet_ntop(AF_INET,&senderAddr.sin_addr, IPtemp, 16), senderAddr.sin_port, recvBuf);
		sendto(recvSocket, recvBuf, BUF_SIZE, 0, (SOCKADDR*)&senderAddr, senderAddrSize);
	}

	printf("Finished receiving. Closeing socket.\n");
	closesocket(recvSocket);
	WSACleanup();
	system("pause");
	return 0;
}