// threadControl.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
using namespace std;

DWORD WINAPI funcA(LPVOID param) {
	while (true) {
		cout << "This is function A\n";
		Sleep(1000);
	}
}

DWORD WINAPI funcB(LPVOID param) {
	while (true) {
		cout << "This is function B\n";
		Sleep(1000);
	}
}

DWORD WINAPI funcC(LPVOID param) {
	while (true) {
		cout << "This is function C\n";
		Sleep(1000);
	}
}

int main(int argc, char* argv[])
{
	char command = NULL;
	DWORD thA = 0, thB = 0, thC = 0;
	bool flag = true;
	HANDLE hd[3];

	hd[0] = CreateThread(NULL, 0, funcA, NULL, CREATE_SUSPENDED, &thA);
	hd[1] = CreateThread(NULL, 0, funcB, NULL, CREATE_SUSPENDED, &thB);
	hd[2] = CreateThread(NULL, 0, funcC, NULL, CREATE_SUSPENDED, &thC);

	cout << "---------------------------------------\n";
	cout << "please input command.\nr: resume all threads\ns : suspend all threads\nt : terminate all threads\nq : quit main process\nh : help\n";
	cout << "------------------------------------\n";

	while (flag) {
		cin >> command;
		switch (command)
		{
		case 'r':
			ResumeThread(hd[0]);
			ResumeThread(hd[1]);
			ResumeThread(hd[2]);
			break;
		case 's':
			SuspendThread(hd[0]);
			SuspendThread(hd[1]);
			SuspendThread(hd[2]);
			break;
		case 't':
			TerminateThread(hd[1], 1);
			TerminateThread(hd[2], 1);
			TerminateThread(hd[0], 1);
			break;
		case 'q':
			flag = false;
			break;
		case 'h':
			cout << "---------------------------------------\n";
			cout << "please input command.\nr: resume all threads\ns : suspend all threads\nt : terminate all threads\nq : quit main process\nh : help\n";
			cout << "---------------------------------------\n"; 
			break;
		default:
			cout << "please input a right command!\n";
			break;
		}
	}

    return 0;
}

