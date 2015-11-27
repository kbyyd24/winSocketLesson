// TcpServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <WINSOCK2.H>   
#include <iostream>
  
#pragma comment(lib,"WS2_32.lib")   
#define BUF_SIZE    64      // ��������С

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
	// �����׽���Ϊ������ģʽ
	/*int iMode = 1;
	retVal = ioctlsocket(m_socket, FIONBIO, (u_long FAR*) &iMode);
	if(retVal == SOCKET_ERROR)
	{
		printf("ioctlsocket failed !\n");
        WSACleanup();   
        return -1;   
	}*/
	
    // ���÷������׽��ֵ�ַ   
    SOCKADDR_IN addrServ;   
    addrServ.sin_family = AF_INET;   
    addrServ.sin_port = htons(9990);		// �����˿�Ϊ9990
    addrServ.sin_addr.S_un.S_addr = htonl(INADDR_ANY);    
    // ���׽���sServer�����ص�ַ���˿�9990  
    retVal = bind(sServer,(const struct sockaddr*)&addrServ,sizeof(SOCKADDR_IN));   
    if(SOCKET_ERROR == retVal)   
    {   
        printf("bind failed !\n");   
        closesocket(sServer);   
        WSACleanup();   
        return -1;   
    }     
    // �����׽���   
    retVal = listen(sServer,1);   
    if(SOCKET_ERROR == retVal)   
    {   
        printf("listen failed !\n");   
        closesocket(sServer);   
        WSACleanup();   
        return -1;   
    }     
    // ���ܿͻ�����   
	bool flag = true;
    printf("TCP Server start...\n");
	while (flag) {
		sockaddr_in addrClient;							// �ͻ��˵�ַ
		int addrClientlen = sizeof(addrClient);
		sClient = accept(sServer, (sockaddr FAR*)&addrClient, &addrClientlen);
		if (INVALID_SOCKET == sClient)
		{
			printf("accept failed !\n");
			closesocket(sServer);
			WSACleanup();
			return -1;
		}

		char *pClientIP = inet_ntoa(addrClient.sin_addr);
		unsigned short clientPort = ntohs(addrClient.sin_port);	// �˿ں����޷�������
		printf("accept a new client...IP: %s	port: %d\n", pClientIP, clientPort);
		//printf("Client connected.\n");
		// ѭ�����տͻ��˵����ݣ�ֱ�ӿͻ��˷���exit���ȷ�Ϻ��˳���  
		while (true)
		{
			ZeroMemory(buf, BUF_SIZE);						// ��ս������ݵĻ�����
			retVal = recv(sClient, buf, BUFSIZ, 0);				// 

			//�ͻ���ǿ�ƹرգ���������������ر�
			if (SOCKET_ERROR == retVal)
			{
				printf("recv failed !\n");
				//closesocket(sServer);
				//closesocket(sClient);
				//WSACleanup();
				//return -1;
				printf("connect will close!\n");
				break;
			}
			// ��ȡ��ǰϵͳʱ��
			SYSTEMTIME st;
			GetLocalTime(&st);
			char sDateTime[30];
			sprintf(sDateTime, "%4d-%2d-%2d %2d:%2d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			// ��ӡ�������Ϣ
			printf("%s, Recv From Client [%s:%d] :%s\n", sDateTime, inet_ntoa(addrClient.sin_addr), addrClient.sin_port, buf);
			// ����ͻ��˷���quit�ַ�������������˳�
			if (strcmp(buf, "logout") == 0)
			{
				retVal = send(sClient, "logout", strlen("logout"), 0);
				break;
			}
			
			//���ڴӿͻ��˹رշ�����������Ҫ�Ļ�����ע�͵�
			if (strcmp(buf, "exit") == 0) {
				retVal = send(sClient, "Do you want to close server? [Y/N]", strlen("Do you want to close server? [y/N]"), 0);
				ZeroMemory(buf, BUF_SIZE);
  				retVal = recv(sClient, buf, BUFSIZ, 0);
				if ('Y' == buf[0]) {
					SYSTEMTIME st;
					GetLocalTime(&st);
					char sDateTime[30];
					sprintf(sDateTime, "%4d-%2d-%2d %2d:%2d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
					// ��ӡ�������Ϣ
					printf("%s, Recv From Client [%s:%d] :%s\n", sDateTime, inet_ntoa(addrClient.sin_addr), addrClient.sin_port, buf);
					retVal = send(sClient, "exit", strlen("exit"), 0);
					flag = false;
					break;
				}
				else if ('N' == buf[0]) {
					SYSTEMTIME st;
					GetLocalTime(&st);
					char sDateTime[30];
					sprintf(sDateTime, "%4d-%2d-%2d %2d:%2d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
					// ��ӡ�������Ϣ
					printf("%s, Recv From Client [%s:%d] :%s\n", sDateTime, inet_ntoa(addrClient.sin_addr), addrClient.sin_port, buf);
					retVal = send(sClient, "won't exit", strlen("won't exit"), 0);
				}
				else {
					retVal = send(sClient, "Unknow input", strlen("Unknow input"), 0);
					//break;
				}
			}
			else		// ������ͻ��˷��ͻ����ַ���
			{
				char    msg[BUF_SIZE];
				sprintf(msg, "Message received - %s", buf);
				retVal = send(sClient, msg, strlen(msg), 0);
				if (SOCKET_ERROR == retVal)
				{
					printf("send failed !\n");
					closesocket(sServer);
					closesocket(sClient);
					WSACleanup();
					return -1;
				}
			}
		}
		// �ͷ��׽���   
		closesocket(sClient);
	}
    closesocket(sServer);   
    WSACleanup();   
	// ��ͣ����������˳�
	system("pause");
	return 0;
}

