// UDPclient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WINSOCK2.H>   
#include <WS2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib,"WS2_32.lib")

#define BUF_SIZE 1024


int main()
{
	WSADATA wsaData;
	SOCKET sendSocket;
	sockaddr_in recvAddr;
	char sendBuf[BUF_SIZE];
	int recvAddrSize = sizeof(recvAddr);
	int res;
	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		printf("WSAStartup() failed with error %d\n", res);
		WSACleanup();
		return -1;
	}
	if ((sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("socket() failed with error %d \n", WSAGetLastError());
		return -1;
	}
	char ip[] = "127.0.0.1";
	char IPtemp[20];
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(10000);
	/*inet_ntop(AF_INET, ip, IPtemp, 16);
	recvAddr.sin_addr.S_un.S_addr = IPtemp;*/
	recvAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	printf("Sending datagrams...\n");

	while (true) {
		printf("input msg:  ");
		std::string str;
		std::getline(std::cin, str);
		// 将用户输入的数据复制到buf中
		ZeroMemory(sendBuf, BUF_SIZE);
		strcpy_s(sendBuf, str.c_str());
		sendto(sendSocket, sendBuf, BUF_SIZE, 0, (SOCKADDR*)&recvAddr, recvAddrSize);
		ZeroMemory(sendBuf, BUF_SIZE);
		recvfrom(sendSocket, sendBuf, BUF_SIZE, 0, (SOCKADDR*)&recvAddr, &recvAddrSize);
		if (strcmp(sendBuf, "quit") == 0) {
			printf("quit");
			break;
		}
		else {
			printf("Recv from server:  %s\n", sendBuf);
		}
	}

	printf("Finished send. closing socket...\n");
	closesocket(sendSocket);
	WSACleanup();
	system("pause");
    return 0;
}

