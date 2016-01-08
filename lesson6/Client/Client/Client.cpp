// Client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <Winsock2.H>   
#include <string>
#include <iostream>

using namespace std;

#pragma comment(lib,"WS2_32.lib")   
#define BUF_SIZE 512
#define OKSTATUS "200"

/*
change value from TCHAR* to char*
*/
void TcharToChar(const TCHAR * tchar, char * _char)
{
	int iLength;
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

int _tmain(int argc, _TCHAR* argv[]) {
	if (3 != argc) {
		printf("error param number");
		return -1;
	}
	WSADATA wsaData;
	SOCKET sHost;
	SOCKADDR_IN servAddr;
	char buf[BUF_SIZE];
	char remoteName[BUF_SIZE], localName[BUF_SIZE];
	int retVal, iResult;
	FILE *fpre;

	TcharToChar(argv[1], remoteName);
	TcharToChar(argv[2], localName);

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(10000);
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int servAddrSize = sizeof(servAddr);
	connect(sHost, (SOCKADDR*)&servAddr, servAddrSize);
	send(sHost, remoteName, BUF_SIZE, 0);
	fpre = fopen(localName, "w");
	printf("Transfering\r\n");
	while (true) {
		retVal = recv(sHost, buf, BUF_SIZE, 0);
		printf(".");
		if (retVal == 0)
			break;
		else if (retVal == SOCKET_ERROR) {
			printf("Receive function failed with error : %d\n ", WSAGetLastError());
			return 1;
		}
		BOOL IS;
		IS = fprintf(fpre, "%s", buf);
		if (IS == -1) {
			printf("Error of writing into the file !");
			return -1;
		}
	}
	fclose(fpre);
	printf("\nTransfered\n");
	shutdown(sHost, SD_BOTH);
	closesocket(sHost);
	WSACleanup();
	system("pause");
}
