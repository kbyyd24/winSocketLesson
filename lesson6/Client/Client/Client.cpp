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

//int downloadThread()
//{
//	WSADATA wsd;			
//	SOCKET sHost;			
//	SOCKADDR_IN servAddr;		
//	char buf[BUF_SIZE];
//	char nameBuf[BUF_SIZE];
//	int retVal, iResult;  
//	FILE *fpre;
//	BOOL issend = true;
//	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
//		printf("WSAStartup failed !\n");
//		return 1;
//	}
//	sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (INVALID_SOCKET == sHost) {
//		printf("socket failed !\n");
//		WSACleanup();
//		return -1;
//	}
//	printf("create socket\n");
//	servAddr.sin_family = AF_INET;
//	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");		
//	servAddr.sin_port = htons(10000);							
//	int sServerAddlen = sizeof(servAddr);						
//
//	if (SOCKET_ERROR == connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr))) {
//			printf("connect failed!\n");
//			closesocket(sHost);
//			WSACleanup();
//			return -1;
//	}
//
//	printf("connection establishment\n");
//	
//	while (true)
//	{
//		printf("input file path on server:\n");
//		string str;
//		getline(cin, str);
//		ZeroMemory(nameBuf, BUF_SIZE);
//		strcpy(nameBuf, str.c_str());
//
//		if (SOCKET_ERROR == send(sHost, nameBuf, BUF_SIZE, 0))
//		{
//			printf("send failed with error : %d\n", WSAGetLastError());
//			closesocket(sHost);
//			WSACleanup();
//			return -1;
//		}
//		ZeroMemory(buf, sizeof(buf));
//		if (SOCKET_ERROR == recv(sHost, buf, sizeof(buf), 0)) {
//			printf("recv failed with error : %d \n", WSAGetLastError());
//			closesocket(sHost);
//			WSACleanup();
//			return -1;
//		}
//		if (strcmp(buf, OKSTATUS) == 0) {
//			printf("input save file path on client:\n");
//			char	CFileName[BUF_SIZE];
//			string str1;
//			getline(cin, str1);
//			strcpy(CFileName, str1.c_str());
//			if ((fpre = fopen(CFileName, "w")) == NULL)
//			{
//				printf("Error of opening file !");
//				return -1;
//			}
//			while (issend)
//			{
//				retVal = recv(sHost, buf, BUF_SIZE, 0);
//				printf(".");
//				if (retVal == 0)
//				{
//					break;
//					//return 1;
//				}
//				else if (retVal == SOCKET_ERROR)
//				{
//					printf("Receive function failed with error : %d\n ", WSAGetLastError());
//					return 1;
//				}
//				BOOL IS;
//				IS = fprintf(fpre, "%s", buf);
//				if (IS == -1)
//				{
//					printf("Error of writing into the file !");
//					return -1;
//				}
//
//			}
//			printf("Transfer is completed!\n\n");
//			fclose(fpre);
//			break;
//		}
//		else {
//			printf("something error : %s\n", buf);
//		}
//	}
//
//
//	iResult = shutdown(sHost, SD_SEND);
//	iResult = closesocket(sHost);
//
//	WSACleanup();
//	return 0;
//}

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
