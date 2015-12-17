// TcpServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WINSOCK2.H>   
#include <iostream>
  
#pragma comment(lib,"WS2_32.lib")   
#define BUF_SIZE    64      // 缓冲区大小
typedef struct _SOCKET_INFOMATION {
	char Buffer[BUF_SIZE];
	SOCKET Socket;
	sockaddr_in addrClient;
} SOCKET_INFOMATION, *LPSOCKET_INFOMATION;

DWORD TotalSockets = 0;
LPSOCKET_INFOMATION SocketArray[FD_SETSIZE];

BOOL createSocketInfo(SOCKET s, sockaddr_in client) {
	LPSOCKET_INFOMATION SI;
	if ((SI = (LPSOCKET_INFOMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFOMATION))) == NULL) {
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return FALSE;
	}
	SI->Socket = s;
	SI->addrClient = client;
	SocketArray[TotalSockets] = SI;
	TotalSockets++;
}

void freeSocketInfo(DWORD Index) {
	LPSOCKET_INFOMATION SI = SocketArray[Index];
	DWORD i;
	closesocket(SI->Socket);
	GlobalFree(SI);
	for (i = Index; i < TotalSockets; i++) {
		SocketArray[i] = SocketArray[i + 1];
	}
	TotalSockets--;
}


int _tmain(int argc, TCHAR* argv[]) {
	SOCKET listenSocket;
	SOCKET acceptSocket;
	SOCKADDR_IN internetAddr;
	WSADATA wsaData;
	INT ret;
	FD_SET writeSet;
	FD_SET readSet;
	DWORD total = 0;
	DWORD sendBytes;
	DWORD recvBytes;

	if ((ret = WSAStartup(0x0202, &wsaData)) != 0) {
		printf("WSAStartup() failed with error %d\n", ret);
		WSACleanup();
		return -1;
	}
	printf("WSAStartup ... \n");

	if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		printf("WSASocket() failed with error %d \n", WSAGetLastError());
		return -1;
	}
	printf("create listen socket \n");

	internetAddr.sin_family = AF_INET;
	internetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	internetAddr.sin_port = htons(10000);

	if (bind(listenSocket, (PSOCKADDR)&internetAddr, sizeof(internetAddr)) == SOCKET_ERROR) {
		printf("bind() failed with error %d\n", WSAGetLastError());
		return -1;
	}
	printf("bind listen addr:any and port:10000\n");

	if (listen(listenSocket, 5)) {
		printf("listen() failed with error %d\n", WSAGetLastError());
		return -1;
	}
	printf("listen socket\n");

	if (!createSocketInfo(listenSocket, internetAddr)) {
		return -1;
	}
	printf("server start ...\n");

	while (true) {
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		FD_SET(listenSocket, &readSet);
		for (DWORD i = 0; i < TotalSockets; i++) {
			LPSOCKET_INFOMATION SI = SocketArray[i];
			FD_SET(SI->Socket, &writeSet);
			FD_SET(SI->Socket, &readSet);
		}
		if ((total = select(0, &readSet, &writeSet, NULL, NULL)) == SOCKET_ERROR) {
			printf("select() failed with error %d\n", WSAGetLastError());
			return -1;
		}

		for (DWORD i = 0; i < TotalSockets; i++) {
			LPSOCKET_INFOMATION SI = SocketArray[i];
			if (FD_ISSET(SI->Socket, &readSet)) {
				if (SI->Socket == listenSocket) {
					total--;
					sockaddr_in clientAddr;
					int addrClientLen = sizeof(clientAddr);
					if ((acceptSocket = accept(listenSocket, (sockaddr FAR*)&clientAddr, &addrClientLen)) != INVALID_SOCKET) {
						if (!createSocketInfo(acceptSocket, clientAddr)) {
							return -1;
						}
						printf("connect socket client [%s:%d]\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
					}
					else {
						printf("accept() failed with error %d \n", WSAGetLastError());
						return -1;
					}
				}
				else {
					total--;
					ZeroMemory(SI->Buffer, BUF_SIZE);

					if (recv(SI->Socket, SI->Buffer, BUF_SIZE, 0) == SOCKET_ERROR) {
						printf("recv() failed with error %d \n", WSAGetLastError());
						freeSocketInfo(i);
						continue;
					}
					else {
						if (strcmp(SI->Buffer, "") == 0) {
							freeSocketInfo(i);
							continue;
						}
						SYSTEMTIME st;
						GetLocalTime(&st);
						char sDateTime[30];
						sprintf(sDateTime, "%4d-%2d-%2d %2d:%2d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
						printf("%s, Recv From Client [%s:%d] :%s\n", sDateTime, inet_ntoa(SI->addrClient.sin_addr), ntohs(SI->addrClient.sin_port), SI->Buffer);
					}
				}
			}
			if (FD_ISSET(SI->Socket, &writeSet)) {
				total--;
				char msg[BUF_SIZE];
				sprintf(msg, "%s", SI->Buffer);
				if (send(SI->Socket, msg, strlen(msg), 0) == SOCKET_ERROR) {
					printf("send() failed with error %d\n", WSAGetLastError());
					freeSocketInfo(i);
				}
				else {
					if (strcmp(SI->Buffer, "logout") == 0) {
						printf("close socket client [%s:%d]\n", inet_ntoa(SI->addrClient.sin_addr), ntohs(SI->addrClient.sin_port));
						freeSocketInfo(i);
						continue;
					}
					ZeroMemory(SI->Buffer, BUF_SIZE);
				}
			}
		}
	}
}

