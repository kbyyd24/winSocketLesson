// chatServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <malloc.h>

#pragma comment(lib, "WS2_32.lib")

#define BUF_SIZE 1024

/*struct of online client*/
typedef struct client {
	sockaddr_in clientAddr;
	char sendBuf[BUF_SIZE];
	//char recvBuf[BUF_SIZE];
	client* next;
	client* pioneer;
}*pClient;

/*struct of msg send to offline client*/
typedef struct msgLine {
	sockaddr_in clientAddr;
	char buf[BUF_SIZE];
	msgLine* next;
}*pMsgLine;

sockaddr_in clientAddr;
pClient clientQueue = NULL;
pClient clientHead = NULL;
pMsgLine msgQueue = NULL;
pMsgLine msgHead = NULL;
int clientAddrSize = sizeof(clientAddr);
char recvBuf[BUF_SIZE];
SOCKET serverSocket;

bool isOnline(SOCKADDR_IN addr) {
	if (clientHead) {
		clientQueue = clientHead;
		while (clientQueue != NULL) {
			if (strcmp(inet_ntoa(addr.sin_addr), inet_ntoa(clientQueue->clientAddr.sin_addr)) == 0) {
				return true;
			}
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
	sockaddr_in cliAddr = *((sockaddr_in*)(LPVOID)param);
	pClient cli;
	clientQueue = clientHead;
	while (clientQueue != NULL) {
		if (strcmp(inet_ntoa(cliAddr.sin_addr), inet_ntoa(clientQueue->clientAddr.sin_addr)) == 0) {
			cli = clientQueue;
		}
		clientQueue = clientQueue->next;
	}
	int cliAddrSize = sizeof(cli->clientAddr);
	clientQueue = clientHead;
	while (clientQueue != NULL) {
		if (strcmp(recvBuf, inet_ntoa(clientQueue->clientAddr.sin_addr)) == 0) {
			//match client, then send OK msg to the sender
			ZeroMemory(cli->sendBuf, BUF_SIZE);
			recvfrom(serverSocket, cli->sendBuf, BUF_SIZE, 0, (SOCKADDR*)&cli->clientAddr, &cliAddrSize);
			char* msg;
			sprintf(msg, "%s send to you %s", inet_ntoa(cli->clientAddr.sin_addr), cli->sendBuf);
			sendto(serverSocket, cli->sendBuf, BUF_SIZE, 0, (SOCKADDR*)&clientQueue->clientAddr, sizeof(clientQueue->clientAddr));
			break;
		}
		clientQueue = clientQueue->next;
	}
	
	return 0;
}

int main()
{
	WSADATA wsaData;
	sockaddr_in recvAddr;

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
	recvAddr.sin_port = htons(9990);
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
						ZeroMemory(clientQueue->sendBuf, BUF_SIZE);
						recvfrom(serverSocket, clientQueue->sendBuf, BUF_SIZE, 0, (SOCKADDR*)&clientQueue->clientAddr, &clientAddrSize);
						char msg[BUF_SIZE];
						sprintf(msg, "%s send to you %s", inet_ntoa(clientQueue->clientAddr.sin_addr), clientQueue->sendBuf);
						sendto(serverSocket, clientQueue->sendBuf, BUF_SIZE, 0, (SOCKADDR*)&clientQueue->clientAddr, sizeof(clientQueue->clientAddr));
						break;
					}
					clientQueue = clientQueue->next;
				}
			}
			else if (strcmp(recvBuf, "logout") == 0) {
				clientQueue = clientHead;
				while (clientQueue != NULL) {
					if (strcmp(inet_ntoa(clientAddr.sin_addr), inet_ntoa(clientQueue->clientAddr.sin_addr)) == 0) {
						//match client, then delete it in online queue
						if (clientQueue->next == NULL) {
							if (clientQueue->pioneer == NULL) {
								free(clientQueue);
								clientHead = NULL;
								clientQueue = clientHead;
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
						break;
					}
					clientQueue = clientQueue->next;
				}
				printf("client %s logout\n", inet_ntoa(clientAddr.sin_addr));
			}
			else {
				char msg[BUF_SIZE];
				sprintf(msg, "server: error command: %s", recvBuf);
				sendto(serverSocket, msg, strlen(msg), 0, (SOCKADDR*)&clientAddr, clientAddrSize);
			}
		}
		else if (strcmp(recvBuf, "login") == 0) {
			//add to client queue
			clientQueue = clientHead;
			if (clientHead == NULL) {
				pClient newClient = (pClient)malloc(sizeof(client));
				clientQueue = newClient;
				clientHead = clientQueue;
				newClient->clientAddr = clientAddr;
				clientHead->pioneer = NULL;
				newClient->next = NULL;
			}
			else {
				while (clientQueue->next != NULL)
					clientQueue = clientQueue->next;
				pClient newClient = (pClient)malloc(sizeof(client));
				clientQueue->next = newClient;
				newClient->pioneer = clientQueue;
				newClient->clientAddr = clientAddr;
				newClient->next = NULL;
			}
			printf("client %s login\n", inet_ntoa(clientAddr.sin_addr));
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

