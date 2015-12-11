// TcpClient.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <Winsock2.H>   
#include <string>
#include <iostream>
  
#pragma comment(lib,"WS2_32.lib")   
#define BUF_SIZE    64          // ��������С  

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA     wsd;					// ���ڳ�ʼ��Windows Socket   
    SOCKET      sHost;					// �����������ͨ�ŵ��׽���   
    SOCKADDR_IN servAddr;			// ��������ַ   
    char        buf[BUF_SIZE];			// ���ڽ������ݻ�����   
    int         retVal;							// ���ø���Socket�����ķ���ֵ   
	// ��ʼ��Windows Socket
    if(WSAStartup(MAKEWORD(2,2),&wsd) != 0)   
    {   
        printf("WSAStartup failed !\n");   
        return 1;   
    }     
    // �����׽���   
    sHost = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);   
    if(INVALID_SOCKET == sHost)   
    {   
        printf("socket failed !\n");   
        WSACleanup();   
        return -1;   
    }
	printf("create socket\n");
	// �����׽���Ϊ������ģʽ
	int iMode = 1;
	retVal = ioctlsocket(sHost, FIONBIO, (u_long FAR*) &iMode);
	if (retVal == SOCKET_ERROR)
	{
		printf("ioctlsocket failed !\n");
		WSACleanup();
		return -1;
	}
    // ���÷�������ַ   
    servAddr.sin_family = AF_INET;   
    servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");		// �û���Ҫ����ʵ������޸�
    servAddr.sin_port = htons(10000);													// ��ʵ��Ӧ���У����齫��������IP��ַ�Ͷ˿ںű����������ļ���
    int sServerAddlen = sizeof(servAddr);												// �����ַ�ĳ���       
    // ���ӷ�����   
	while (true) {
		retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
		if (SOCKET_ERROR == retVal) {
			int err = WSAGetLastError();
			if (WSAEWOULDBLOCK == err || WSAEINVAL == err) {
				continue;
			}
			else if (err == WSAEISCONN) {
				break;
			}
			else {
				printf("connect failed!\n");
				closesocket(sHost);
				WSACleanup();
				return -1;
			}
		}
	}
	printf("connection establishment\n");
	// ѭ���ȴ�
	while(true)
	{
		if(INVALID_SOCKET == sHost)   
		{   
			int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK)			// �޷�������ɷ������׽����ϵĲ���
			{
				//Sleep(500);
				continue;
			}
			else
			{
				printf("accept failed !\n");   
				closesocket(sHost);   
				WSACleanup();   
				return -1;   
			}
		}
		break;
	}  
	
	// ѭ��������������ַ���������ʾ������Ϣ��
	// ����quit��ʹ�����������˳���ͬʱ�ͻ��˳�������Ҳ���˳�
	while(true)
	{
		// ���������������   
		printf("Please input a string to send: ");
		// �������������
		std::string str;
		std::getline(std::cin, str);
		// ���û���������ݸ��Ƶ�buf��
		ZeroMemory(buf,BUF_SIZE);   
		strcpy(buf,str.c_str());   
		// ���������������
		retVal = send(sHost,buf,strlen(buf),0);   
		if(SOCKET_ERROR == retVal)   
		{   
			printf("send failed !\n");   
			closesocket(sHost);   
			WSACleanup();   
			return -1;   
		}   	  
		// ���շ������ش�������   
		retVal = recv(sHost,buf,sizeof(buf)+1,0);   
		printf("Recv From Server: %s\n",buf);   
		// ����յ�logout�����˳�
		if(strcmp(buf, "logout") == 0)
		{
			printf("logout!\n");
			break;
		}
		else if (strcmp(buf, "exit") == 0) {
			printf("exit!\n");
			break;
		}
	}
	printf("release connection\n");
	printf("release source\n");
    // �ͷ���Դ   
    closesocket(sHost);   
    WSACleanup();   
	// ��ͣ�������������
	system("pause");
    return 0;  
}

