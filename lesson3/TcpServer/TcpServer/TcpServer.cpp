// TcpServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WINSOCK2.H>   
#include <iostream>
  
#pragma comment(lib,"WS2_32.lib")   
#define BUF_SIZE    64      // 缓冲区大小
sockaddr_in addrClient;


DWORD WINAPI connectThread(LPVOID param) {
	char buf[BUF_SIZE];
	int retVal;
	SOCKET sClient = (SOCKET)(LPVOID)param;

	char *pClientIP = inet_ntoa(addrClient.sin_addr);
	unsigned short clientPort = ntohs(addrClient.sin_port);	// 端口号是无符号整型
	printf("accept a new client...IP: %s	port: %d\n", pClientIP, clientPort);
	printf("Client connected.\n");
	// 循环接收客户端的数据，直接客户端发送exit命令并确认后退出。  
	while (true)
	{
		ZeroMemory(buf, BUF_SIZE);						// 清空接收数据的缓冲区
		retVal = recv(sClient, buf, BUFSIZ, 0);				// 

															//客户端强制关闭，这样服务器不会关闭
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
		// 获取当前系统时间
		SYSTEMTIME st;
		GetLocalTime(&st);
		char sDateTime[30];
		sprintf(sDateTime, "%4d-%2d-%2d %2d:%2d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		// 打印输出的信息
		printf("%s, Recv From Client [%s:%d] :%s\n", sDateTime, inet_ntoa(addrClient.sin_addr), clientPort, buf);
		// 如果客户端发送logout字符串，则客户端退出
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
	// 释放套接字   
	printf("release connection\n");
	closesocket(sClient);
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsd;					// WSADATA变量，用于初始化Windows Socket   
    SOCKET  sServer;					// 服务器套接字，用于监听客户端请求
    SOCKET  sClient;					// 客户端套接字，用于实现与客户端的通信   
    int     retVal;							// 调用各种Socket函数的返回值   
    char    buf[BUF_SIZE];			// 用于接受客户端数据的缓冲区   
  
    // 初始化套接字动态库   
    if(WSAStartup(MAKEWORD(2,2),&wsd) != 0)   
    {   
        printf("WSAStartup failed !\n");   
        return 1;   
    }     
    // 创建用于监听的套接字   
    sServer = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);   
    if(INVALID_SOCKET == sServer)   
    {   
        printf("socket failed !\n");   
        WSACleanup();   
        return -1;   
    }
	printf("create socket!\n");
	// 设置套接字为非阻塞模式
	int iMode = 1;
	retVal = ioctlsocket(sServer, FIONBIO, (u_long FAR*) &iMode);
	if(retVal == SOCKET_ERROR)
	{
		printf("ioctlsocket failed !\n");
        WSACleanup();   
        return -1;   
	}
	
    // 设置服务器套接字地址   
    SOCKADDR_IN addrServ;
    addrServ.sin_family = AF_INET;   
    addrServ.sin_port = htons(10000);		// 监听端口为10000
    addrServ.sin_addr.S_un.S_addr = htonl(INADDR_ANY);    
    // 绑定套接字sServer到本地地址，端口10000  
    retVal = bind(sServer,(const struct sockaddr*)&addrServ,sizeof(SOCKADDR_IN));   
    if(SOCKET_ERROR == retVal)   
    {   
        printf("bind failed !\n");   
        closesocket(sServer);   
        WSACleanup();   
        return -1;   
    }
	printf("bind port 10000\n");
    // 监听套接字   
    retVal = listen(sServer,1);   
    if(SOCKET_ERROR == retVal)   
    {   
        printf("listen failed !\n");   
        closesocket(sServer);   
        WSACleanup();   
        return -1;   
    }
	printf("listen port 10000\n");
    // 接受客户请求
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
	// 暂停，按任意键退出
	system("pause");
	return 0;
}

