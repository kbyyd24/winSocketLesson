// my_ping.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include <iostream>
#include <string>

#pragma comment(lib, "WS2_32.lib")
#define ICMP_ECHOREPLY 0
#define ICMP_ECHOREQ 8
#define ICMP_MIN 8
#define MAX_PACKET 1024
#define DEF_PAKCET_SIZE 32

/*
IP packet header struct
  h_len:          length of header
  version:        IP version
  tos:            type of service
  total_len:      length of packet
  ident:          packet's only identification
  frag_and_flags: identification
  ttl:            time to live
  proto:          protocal
  checksum:       IP's check sum
  sourceIP:       source IP
  destIP:         destination IP
*/
typedef struct iphdr {
	unsigned int   h_len : 4;
	unsigned int   version : 4;
	unsigned char  tos;
	unsigned short total_len;
	unsigned short ident;
	unsigned short frag_and_flags;
	unsigned char  ttl;
	unsigned char  proto;
	unsigned short checksum;
	unsigned int   sourceIP;
	unsigned int   destIP;
}IpHeader;

/*
the ICMP packet struct that is included in IP packet which is sended when you do ping program
  i_type:    type of icmp
  i_code:    code of icmp
  i_cksum:   check sum of icmp
  i_id:      id of icmp
  i_seq:     sequence number
  timestamp: timestamp
*/
typedef struct _ihdr {
	BYTE   i_type;
	BYTE   i_code;
	USHORT i_cksum;
	USHORT i_id;
	USHORT i_seq;
	ULONG  timestamp;
}IcmpHeader;

/*
fill icmp packet
*/
void fill_icmp_data(char* icmp_data, int datasize) {
	IcmpHeader *icmp_hdr;
	char *datapart;
	icmp_hdr = (IcmpHeader*)icmp_data;
	icmp_hdr->i_type = ICMP_ECHOREQ;
	icmp_hdr->i_code = 0;
	icmp_hdr->i_id = (USHORT)GetCurrentThreadId();
	icmp_hdr->i_cksum = 0;
	icmp_hdr->i_seq = 0;
	datapart = icmp_data + sizeof(IcmpHeader);
	memset(datapart, 'E', datasize - sizeof(IcmpHeader));
}

/*
decode the return IP packet, locate at ICMP packet
  buf:   receive buffer
  bytes: buffer length
  from:  packet sender's IP
  tid:   thread's ID which receive ICMP packet
*/
int decode_resp(char *buf, int bytes, struct sockaddr_in *from, DWORD tid) { 
	IpHeader *iphdr;
	IcmpHeader *icmphdr;
	unsigned short iphdrlen;
	iphdr = (IpHeader*)buf;
	iphdrlen = iphdr->h_len * 4;
	if (bytes < iphdrlen + ICMP_MIN) {
		return -1;
	}
	icmphdr = (IcmpHeader*)(buf + iphdrlen);
	if (icmphdr->i_type != ICMP_ECHOREPLY) {
		return -2;
	}
	if (icmphdr->i_id != (USHORT)tid) {
		return -3;
	}
	int time = GetTickCount() - (icmphdr->timestamp);
	if (time >= 0) return time;
	else return -4;
}

/*
check the checksum in ICMP packet
*/
USHORT checksum(USHORT *buffer, int size)
{
	unsigned long cksum = 0;
	while (size >1) {
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}
	if (size) {
		cksum += *(UCHAR*)buffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (USHORT)(~cksum);
}

/*
ping operation method
  ip:      destination IP
  timeout: time of timeout
*/
int ping(const char* ip, DWORD timeout) {
	WSADATA wsaData;
	SOCKET sockRaw;
	sockaddr_in dest, from;
	hostent *hp;
	int datasize;
	char *icmp_data, *recvbuf;
	USHORT seq_no = 0;
	int ret = -1;

	if (WSAStartup(MAKEWORD(2,1), &wsaData) != 0) {
		ret = -1000;
		goto FIN;
	}
	sockRaw = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//sockRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (INVALID_SOCKET == sockRaw) {
		printf("socket failed with : %d\n", WSAGetLastError());
		ret = -2;
		goto FIN;
	}
	int bread = setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	if (bread == SOCKET_ERROR) {
		ret = -3;
		goto FIN;
	}
	bread = setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	if (bread == SOCKET_ERROR) {
		ret = -4;
		goto FIN;
	}
	memset(&dest, 0, sizeof(dest));

	unsigned int addr = 0;
	hp = gethostbyname(ip);
	if (!hp) {
		addr = inet_addr(ip);
		if (addr == INADDR_NONE) {
			ret = -5;
			goto FIN;
		}
	}
	if (hp != NULL)
		memcpy(&(dest.sin_addr), hp->h_addr_list[0], hp->h_length);
	else
		dest.sin_addr.S_un.S_addr = addr;

	if (hp)
		dest.sin_family = hp->h_addrtype;
	else
		dest.sin_family = AF_INET;
	char* dest_ip = inet_ntoa(dest.sin_addr);
	from = dest;

	datasize = DEF_PAKCET_SIZE;
	datasize += sizeof(IcmpHeader);
	char icmp_dataStack[MAX_PACKET];
	char recvbufStack[MAX_PACKET];
	icmp_data = icmp_dataStack;
	recvbuf = recvbufStack;
	if (!icmp_data) {
		ret = -6;
		goto FIN;
	}
	memset(icmp_data, 0, MAX_PACKET);

	fill_icmp_data(icmp_data, datasize);
	((IcmpHeader*)icmp_data)->i_cksum = 0;//?
	DWORD startTime = GetTickCount();
	((IcmpHeader*)icmp_data)->timestamp = startTime;
	((IcmpHeader*)icmp_data)->i_seq = seq_no++;
	((IcmpHeader*)icmp_data)->i_cksum = checksum((USHORT*)icmp_data, datasize);

	int bwrote;
	bwrote = sendto(sockRaw, icmp_data, datasize, 0, (sockaddr*)&dest, sizeof(dest));
	if (bwrote == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAETIMEDOUT) {
			ret = -7;
			goto FIN;
		}
	}
	if (bwrote < datasize) {
		ret = -8;
		goto FIN;
	}

	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_tick;
	LARGE_INTEGER end_tick;
	double elapsed;
	QueryPerformanceFrequency(&ticksPerSecond);
	QueryPerformanceCounter(&start_tick);
	int fromlen = sizeof(from);
	while (true) {
		bread = recvfrom(sockRaw, recvbuf, MAX_PACKET, 0, (sockaddr*)&from, &fromlen);
		if (bread == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				ret = -1;
				goto FIN;
			}
			ret = -9;
			goto FIN;
		}
		int time = decode_resp(recvbuf, bread, &from, GetCurrentThreadId());
		if (time >= 0) {
			QueryPerformanceCounter(&end_tick);
			elapsed = ((double)(end_tick.QuadPart - start_tick.QuadPart) / ticksPerSecond.QuadPart);
			ret = (int)(elapsed * 1000);
			goto FIN;
		}
		else if (GetTickCount() - startTime >= timeout || GetTickCount() < startTime) {
			ret = -1;
			goto FIN;
		}
	}
FIN:
	closesocket(sockRaw);
	WSACleanup();
	return ret;
}

/*
change value from TCHAR* to char*
*/
void TcharToChar(const TCHAR * tchar, char * _char)
{
	int iLength;
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2) {
		printf("ERROR: error number of parameter, please input:\n  my_ping <IP address>\n-----------------------------------------------\n");
		return -1;
	}
	char iptemp[16];
	TcharToChar(argv[1], iptemp);
	printf("ping %ws...\n", argv[1]);
	int ret = ping(iptemp, 500);
	if (ret >= 0) {
		printf("%s is online, operation ping used %dms.\n", argv[1], ret);
	}
	else {
		switch (ret) {
		case -1:
			printf("timeout.\n");
			break;
		case -2:
			printf("create socket error.\n");
			break;
		case -3:
			printf("set receive timeout error.\n");
			break;
		case -4:
			printf("set send timeout error.\n");
			break;
		case -5:
			printf("get domain name error.\n");
			break;
		case -6:
			printf("ICMP packet doesn't have enough space.\n");
			break;
		case -7:
			printf("send packet error.\n");
			break;
		case -8:
			printf("error number of packet");
			break;
		case -9:
			printf("receive packet error.\n");
			break;
		case -1000:
			printf("initialize Windows Sockets environment error.\n");
			break;
		default:
			printf("unknow error.\n");
			break;
		}
	}
	printf("\n");
	system("pause");
    return 0;
}

