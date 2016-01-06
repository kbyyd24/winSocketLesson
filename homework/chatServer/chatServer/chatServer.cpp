// chatServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <malloc.h>

#pragma comment(lib, "WS2_32.lib")

#define BUF_SIZE 1024
#define LOGINMSG "OK"
#define MATCH "MATCH"
#define IPERROR "IPERROR"

/*struct of online client*/
typedef struct client {
	sockaddr_in clientAddr;
	char sendBuf[BUF_SIZE];
	//char recvBuf[BUF_SIZE];
	client* next = NULL;
	client* pioneer = NULL;
}*pClient;

/*struct of msg send to offline client*/
typedef struct msgLine {
	sockaddr_in clientAddr;
	char buf[BUF_SIZE];
	msgLine* next = NULL;
}*pMsgLine;

sockaddr_in clientAddr;
pClient clientQueue;
pClient clientHead;
pMsgLine msgQueue;
pMsgLine msgHead;
int clientAddrSize = sizeof(clientAddr);
SOCKET serverSocket;

bool isOnline(SOCKADDR_IN addr) {
	clientQueue = clientHead;
	while (clientQueue != NULL) {
		if (strcmp(inet_ntoa(addr.sin_addr), inet_ntoa(clientQueue->clientAddr.sin_addr)) == 0) {
			return true;
		}
	}
	return false;
}

bool isIp(char *ipaddr)
{
	char *pnum, *pdot = ipaddr;
	for (; *ipaddr; ipaddr = pdot++)
	{
		int t = 0, e = 1;
		if (*(pnum = pdot) == '.')return 0;
		for (; *pdot != '.'&&*pdot; ++pdot);
		for (ipaddr = pdot - 1; ipaddr >= pnum; t += e*(*ipaddr-- - '0'), e *= 10);
		if (t<0 || t>255 || (pdot - pnum == 3 && t<100) || (pdot - pnum == 2 && t<10))
			return false;
	}
	return true;
}

DWORD WINAPI service(LPVOID param) {
	pClient cli = (pClient)(LPVOID)param;
	sendto(serverSocket, MATCH, strlen(MATCH), 0, (SOCKADDR*)&clientAddr, clientAddrSize);
	ZeroMemory(cli->sendBuf, BUF_SIZE);
	recvfrom(serverSocket, cli->sendBuf, BUF_SIZE, 0, (SOCKADDR*)&clientAddr, &clientAddrSize);
	sendto(serverSocket, cli->sendBuf, BUF_SIZE, 0, (SOCKADDR*)&clientQueue->clientAddr, sizeof(clientQueue->clientAddr));
	return 0;
}

int main()
{
	WSADATA wsaData;
	sockaddr_in recvAddr;
	char recvBuf[BUF_SIZE];

	clientQueue = (pClient)malloc(sizeof(client));
	msgQueue = (pMsgLine)malloc(sizeof(msgLine));
	clientHead = clientQueue;
	msgHead = msgQueue;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	if ((serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("socket failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -2;
	}
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(10000);
	recvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind(serverSocket, (sockaddr*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR) {
		printf("bind failed with error %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return -3;
	}
	printf("Server startup.\n");

	while (true) {
		ZeroMemory(recvBuf, BUF_SIZE);
		if (SOCKET_ERROR == recvfrom(serverSocket, recvBuf, BUF_SIZE, 0, (SOCKADDR*)&clientAddr, &clientAddrSize))
			continue;
		if (isOnline(clientAddr)) {
			//check recvBuf
			if (isIp(recvBuf)) {
				clientQueue = clientHead;
				while (clientQueue != NULL) {
					if (strcmp(recvBuf, inet_ntoa(clientQueue->clientAddr.sin_addr)) == 0) {
						//match client, then send OK msg to the sender
						DWORD thId;
						CreateThread(NULL, NULL, service, (LPVOID)clientQueue, 0, &thId);
						break;
					}
					clientQueue = clientQueue->next;
				}
			}
			else if (strcmp(recvBuf, "logout") == 0) {
				clientQueue = clientHead;
				while (clientQueue != NULL) {
					if (strcmp(recvBuf, inet_ntoa(clientQueue->clientAddr.sin_addr)) == 0) {
						//match client, then delete it in online queue
						if (clientQueue->next == NULL) {
							if (clientQueue->pioneer == NULL) {
								free(clientQueue);
								free(clientHead);
							}
							else {
								clientQueue->pioneer->next = NULL;
								free(clientQueue);
							}
						}
						else if (clientQueue->pioneer == NULL) {
							clientHead = clientHead->next;
							free(clientQueue);
							clientHead->pioneer = NULL;
						}
						else {
							clientQueue->pioneer->next = clientQueue->next;
							clientQueue->next->pioneer = clientQueue->pioneer;
							free(clientQueue);
						}
					}
					clientQueue = clientQueue->next;
				}
			}
			else {
				sendto(serverSocket, IPERROR, strlen(IPERROR), 0, (SOCKADDR*)&clientAddr, clientAddrSize);
			}
		}
		else if (strcmp(recvBuf, "login") == 0) {
			//add to client queue
			clientQueue = clientHead;
			while (clientQueue->next != NULL)
				clientQueue = clientQueue->next;
			pClient newClient = (pClient)malloc(sizeof(client));
			clientQueue->next = newClient;
			newClient->pioneer = clientQueue;
			newClient->clientAddr = clientAddr;
			sendto(serverSocket, LOGINMSG, strlen(LOGINMSG), 0, (SOCKADDR*)&(newClient->clientAddr), sizeof(newClient->clientAddr));
		}
		else {
			//add to message queue
		}
	}

	printf("Server stop.\n");
	closesocket(serverSocket);
	WSACleanup();
	system("pause");
    return 0;
}

