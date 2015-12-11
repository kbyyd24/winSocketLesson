// TcpClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <Winsock2.H>   
#include <string>
#include <iostream>
  
#pragma comment(lib,"WS2_32.lib")   
#define BUF_SIZE    64          // 缓冲区大小  

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA     wsd;					// 用于初始化Windows Socket   
    SOCKET      sHost;					// 与服务器进行通信的套接字   
    SOCKADDR_IN servAddr;			// 服务器地址   
    char        buf[BUF_SIZE];			// 用于接受数据缓冲区   
    int         retVal;							// 调用各种Socket函数的返回值   
	// 初始化Windows Socket
    if(WSAStartup(MAKEWORD(2,2),&wsd) != 0)   
    {   
        printf("WSAStartup failed !\n");   
        return 1;   
    }     
    // 创建套接字   
    sHost = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);   
    if(INVALID_SOCKET == sHost)   
    {   
        printf("socket failed !\n");   
        WSACleanup();   
        return -1;   
    }
	printf("create socket\n");
	// 设置套接字为非阻塞模式
	int iMode = 1;
	retVal = ioctlsocket(sHost, FIONBIO, (u_long FAR*) &iMode);
	if (retVal == SOCKET_ERROR)
	{
		printf("ioctlsocket failed !\n");
		WSACleanup();
		return -1;
	}
    // 设置服务器地址   
    servAddr.sin_family = AF_INET;   
    servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");		// 用户需要根据实际情况修改
    servAddr.sin_port = htons(10000);													// 在实际应用中，建议将服务器的IP地址和端口号保存在配置文件中
    int sServerAddlen = sizeof(servAddr);												// 计算地址的长度       
    // 连接服务器   
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
	// 循环等待
	while(true)
	{
		if(INVALID_SOCKET == sHost)   
		{   
			int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK)			// 无法立即完成非阻塞套接字上的操作
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
	
	// 循环向服务器发送字符串，并显示反馈信息。
	// 发送quit将使服务器程序退出，同时客户端程序自身也将退出
	while(true)
	{
		// 向服务器发送数据   
		printf("Please input a string to send: ");
		// 接收输入的数据
		std::string str;
		std::getline(std::cin, str);
		// 将用户输入的数据复制到buf中
		ZeroMemory(buf,BUF_SIZE);   
		strcpy(buf,str.c_str());   
		// 向服务器发送数据
		retVal = send(sHost,buf,strlen(buf),0);   
		if(SOCKET_ERROR == retVal)   
		{   
			printf("send failed !\n");   
			closesocket(sHost);   
			WSACleanup();   
			return -1;   
		}   	  
		// 接收服务器回传的数据   
		retVal = recv(sHost,buf,sizeof(buf)+1,0);   
		printf("Recv From Server: %s\n",buf);   
		// 如果收到logout，则退出
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
    // 释放资源   
    closesocket(sHost);   
    WSACleanup();   
	// 暂停，按任意键继续
	system("pause");
    return 0;  
}

