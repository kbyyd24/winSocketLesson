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

int downloadThread()
{
	WSADATA wsd;			
	SOCKET sHost;			
	SOCKADDR_IN servAddr;		
	char buf[BUF_SIZE];
	char nameBuf[BUF_SIZE];
	int retVal, iResult;  
	FILE *fpre;
	BOOL issend = true;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
		printf("WSAStartup failed !\n");
		return 1;
	}
	sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sHost) {
		printf("socket failed !\n");
		WSACleanup();
		return -1;
	}
	printf("create socket\n");
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");		
	servAddr.sin_port = htons(10000);							
	int sServerAddlen = sizeof(servAddr);						

	if (SOCKET_ERROR == connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr))) {
			printf("connect failed!\n");
			closesocket(sHost);
			WSACleanup();
			return -1;
	}

	printf("connection establishment\n");
	
	while (true)
	{
		printf("input file path on server:\n");
		string str;
		getline(cin, str);
		ZeroMemory(nameBuf, BUF_SIZE);
		strcpy(nameBuf, str.c_str());

		if (SOCKET_ERROR == send(sHost, nameBuf, BUF_SIZE, 0))
		{
			printf("send failed with error : %d\n", WSAGetLastError());
			closesocket(sHost);
			WSACleanup();
			return -1;
		}
		ZeroMemory(buf, sizeof(buf));
		if (SOCKET_ERROR == recv(sHost, buf, sizeof(buf), 0)) {
			printf("recv failed with error : %d \n", WSAGetLastError());
			closesocket(sHost);
			WSACleanup();
			return -1;
		}
		if (strcmp(buf, OKSTATUS) == 0) {
			printf("input save file path on client:\n");
			char	CFileName[BUF_SIZE];
			string str1;
			getline(cin, str1);
			strcpy(CFileName, str1.c_str());
			if ((fpre = fopen(CFileName, "w")) == NULL)
			{
				printf("Error of opening file !");
				return -1;
			}
			while (issend)
			{
				retVal = recv(sHost, buf, BUF_SIZE, 0);
				printf(".");
				if (retVal == 0)
				{
					break;
					//return 1;
				}
				else if (retVal == SOCKET_ERROR)
				{
					printf("Receive function failed with error : %d\n ", WSAGetLastError());
					return 1;
				}
				BOOL IS;
				IS = fprintf(fpre, "%s", buf);
				if (IS == -1)
				{
					printf("Error of writing into the file !");
					return -1;
				}

			}
			printf("Transfer is completed!\n\n");
			fclose(fpre);
			break;
		}
		else {
			printf("something error : %s\n", buf);
		}
	}


	iResult = shutdown(sHost, SD_SEND);
	iResult = closesocket(sHost);

	WSACleanup();
	return 0;
}


int _tmain(int argc, _TCHAR* argv[]) {
	while (true) {
		printf("select one thing to do: \n\t0-download\n\t1-logout\n");
		char input[8];
		string str;
		getline(cin, str);
		ZeroMemory(input, sizeof(input));
		strcpy(input, str.c_str());
		if (strcmp(input, "0") == 0) {
			downloadThread();
		}
		else if (strcmp(input, "1") == 0) {
			printf("bye!\n");
			break;
		}
		else {
			printf("please input a right number\n");
		}
	}
	system("pause");
}
