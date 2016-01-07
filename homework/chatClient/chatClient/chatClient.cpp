// chatClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WINSOCK2.H>   
#include <WS2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib,"WS2_32.lib")

#define BUF_SIZE 1024
#define MAX_LINE 10

SOCKET clientSocket;
SOCKADDR_IN recvAddr;
char recvMsg[MAX_LINE][BUF_SIZE];
int totalMsg = 0;

DWORD WINAPI recvThread(LPVOID param) {
	int addrSize = *((int*)(LPVOID)param);
	while (true) {
		recvfrom(clientSocket, recvMsg[totalMsg], BUF_SIZE, 0, (SOCKADDR*)&recvAddr, &addrSize);
		totalMsg++;
		if (totalMsg == MAX_LINE) {
			printf("receive queue is full\n");
			break;
		}
	}
	return 0;
}

int main()
{
	WSADATA wsaData;
	char sendBuf[BUF_SIZE];
	int recvAddrSize = sizeof(recvAddr);
	char ip[16];

	printf("input server's ip: ");
	std::string str1;
	std::getline(std::cin, str1);
	sprintf(ip, "%s", str1.c_str());

	for (int i = 0; i < MAX_LINE; i++)
		ZeroMemory(recvMsg[i], BUF_SIZE);
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(9990);
	recvAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	DWORD thId;
	HANDLE thhandle;

	printf("client startup\n");
	sendto(clientSocket, "login", strlen("login"), 0, (SOCKADDR*)&recvAddr, recvAddrSize);
	while (true) {
		thhandle = CreateThread(NULL, NULL, recvThread, (LPVOID)&recvAddrSize, 0, &thId);
		printf("\t1.send Msg 2.receive Msg 0.logout\n");
		std::string str;
		std::getline(std::cin, str);
		if (strcmp(str.c_str(), "1") == 0) {
			printf("input destination IP: ");
			std::getline(std::cin, str);
			ZeroMemory(sendBuf, BUF_SIZE);
			strcpy(sendBuf, str.c_str());
			sendto(clientSocket, sendBuf, BUF_SIZE, 0, (SOCKADDR*)&recvAddr, recvAddrSize);
			ZeroMemory(sendBuf, BUF_SIZE);
			/*recvfrom(clientSocket, sendBuf, BUF_SIZE, 0, (SOCKADDR*)&recvAddr, &recvAddrSize);
			if (strcmp(sendBuf, "MATCH") == 0) {*/
				printf("input the message: ");
				std::getline(std::cin, str);
				ZeroMemory(sendBuf, BUF_SIZE);
				strcpy(sendBuf, str.c_str());
				sendto(clientSocket, sendBuf, BUF_SIZE, 0, (SOCKADDR*)&recvAddr, recvAddrSize);
			//}
		}
		else if (strcmp(str.c_str(), "2") == 0) {
			if (totalMsg > 0) {
				for (int i = 0; i < totalMsg; i++) {
					printf("msg %d : %s\n", i, recvMsg[i]);
					ZeroMemory(recvMsg[i], BUF_SIZE);
				}
				totalMsg = 0;
			}
		}
		else if (strcmp(str.c_str(), "0") == 0) {
			sendto(clientSocket, "logout", strlen("logout"), 0, (SOCKADDR*)&recvAddr, recvAddrSize);
			break;
		}
	}
	TerminateThread(thhandle, thId);
	system("pause");
    return 0;
}

