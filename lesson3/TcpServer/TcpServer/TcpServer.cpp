// TcpServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <WINSOCK2.H>   
#include <iostream>
  
#pragma comment(lib,"WS2_32.lib")   
#define BUF_SIZE    64      // ��������С
sockaddr_in addrClient;


DWORD WINAPI connectThread(LPVOID param) {
	char buf[BUF_SIZE];
	int retVal;
	SOCKET sClient = (SOCKET)(LPVOID)param;

	char *pClientIP = inet_ntoa(addrClient.sin_addr);
	unsigned short clientPort = ntohs(addrClient.sin_port);	// �˿ں����޷�������
	printf("accept a new client...IP: %s	port: %d\n", pClientIP, clientPort);
	printf("Client connected.\n");
	// ѭ�����տͻ��˵����ݣ�ֱ�ӿͻ��˷���exit���ȷ�Ϻ��˳���  
	while (true)
	{
		ZeroMemory(buf, BUF_SIZE);						// ��ս������ݵĻ�����
		retVal = recv(sClient, buf, BUFSIZ, 0);				// 

															//�ͻ���ǿ�ƹرգ���������������ر�
		if (SOCKET_ERROR == retVal)
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
				continue;
			else {
				printf("recv failed !\n");
				printf("connect will close!\n");
				break;
			}
		}
		// ��ȡ��ǰϵͳʱ��
		SYSTEMTIME st;
		GetLocalTime(&st);
		char sDateTime[30];
		sprintf(sDateTime, "%4d-%2d-%2d %2d:%2d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		// ��ӡ�������Ϣ
		printf("%s, Recv From Client [%s:%d] :%s\n", sDateTime, inet_ntoa(addrClient.sin_addr), clientPort, buf);
		// ����ͻ��˷���logout�ַ�������ͻ����˳�
		if (strcmp(buf, "logout") == 0)
		{
			retVal = send(sClient, "logout", strlen("logout"), 0);
			break;
		}
		char    msg[BUF_SIZE];
		sprintf(msg, "Message received - %s", buf);
		retVal = send(sClient, msg, strlen(msg), 0);
		if (SOCKET_ERROR == retVal)
		{
			printf("send failed !\n");
			closesocket(sClient);
			return -1;
		}
	}
	// �ͷ��׽���   
	printf("release connection\n");
	closesocket(sClient);
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsd;					// WSADATA���������ڳ�ʼ��Windows Socket   
    SOCKET  sServer;					// �������׽��֣����ڼ����ͻ�������
    SOCKET  sClient;					// �ͻ����׽��֣�����ʵ����ͻ��˵�ͨ��   
    int     retVal;							// ���ø���Socket�����ķ���ֵ   
    char    buf[BUF_SIZE];			// ���ڽ��ܿͻ������ݵĻ�����   
  
    // ��ʼ���׽��ֶ�̬��   
    if(WSAStartup(MAKEWORD(2,2),&wsd) != 0)   
    {   
        printf("WSAStartup failed !\n");   
        return 1;   
    }     
    // �������ڼ������׽���   
    sServer = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);   
    if(INVALID_SOCKET == sServer)   
    {   
        printf("socket failed !\n");   
        WSACleanup();   
        return -1;   
    }
	printf("create socket!\n");
	// �����׽���Ϊ������ģʽ
	int iMode = 1;
	retVal = ioctlsocket(sServer, FIONBIO, (u_long FAR*) &iMode);
	if(retVal == SOCKET_ERROR)
	{
		printf("ioctlsocket failed !\n");
        WSACleanup();   
        return -1;   
	}
	
    // ���÷������׽��ֵ�ַ   
    SOCKADDR_IN addrServ;
    addrServ.sin_family = AF_INET;   
    addrServ.sin_port = htons(10000);		// �����˿�Ϊ10000
    addrServ.sin_addr.S_un.S_addr = htonl(INADDR_ANY);    
    // ���׽���sServer�����ص�ַ���˿�10000  
    retVal = bind(sServer,(const struct sockaddr*)&addrServ,sizeof(SOCKADDR_IN));   
    if(SOCKET_ERROR == retVal)   
    {   
        printf("bind failed !\n");   
        closesocket(sServer);   
        WSACleanup();   
        return -1;   
    }
	printf("bind port 10000\n");
    // �����׽���   
    retVal = listen(sServer,1);   
    if(SOCKET_ERROR == retVal)   
    {   
        printf("listen failed !\n");   
        closesocket(sServer);   
        WSACleanup();   
        return -1;   
    }
	printf("listen port 10000\n");
    // ���ܿͻ�����
	bool flag = true;
    printf("TCP Server start...\n");
	while (flag) {
		int addrClientlen = sizeof(addrClient);
		while (true) {
			sClient = accept(sServer, (sockaddr FAR*)&addrClient, &addrClientlen);
			if (INVALID_SOCKET == sClient)
			{
				int err = WSAGetLastError();
				if (err == WSAEWOULDBLOCK)
					continue;
				else {
					printf("accept failed !\n");
					closesocket(sServer);
					WSACleanup();
					return -1;
				}
			}
			DWORD dwThreadID;
			CreateThread(NULL, NULL, connectThread, (LPVOID)sClient, 0, &dwThreadID);
		}
		
	}
    closesocket(sServer);   
    WSACleanup();   
	printf("release source\n");
	// ��ͣ����������˳�
	system("pause");
	return 0;
}

