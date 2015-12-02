// criticalSectionDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
using namespace std;

int tickets = 10;
HANDLE evFin;
CRITICAL_SECTION cs;

DWORD WINAPI thProc1(LPVOID param) {
	while (true) {
		EnterCriticalSection(&cs);
		if (tickets > 0) {
			tickets--;
			cout << "thread one sell one ticket\nyou have " << tickets << " tickets now.\n";
		}
		else {
			cout << "no enough ticket 1\n";
			LeaveCriticalSection(&cs);
			Sleep(1);
			SetEvent(evFin);
			break;
		}
		LeaveCriticalSection(&cs);
		Sleep(1);
		//SetEvent(evFin);
	}
	return 0;
}

DWORD WINAPI thProc2(LPVOID param) {
	while (true) {
		EnterCriticalSection(&cs);
		if (tickets > 0) {
			tickets--;
			cout << "thread two sell one ticket\nyou have " << tickets << " tickets now.\n";
		}
		else {
			cout << "no enough ticket 2\n";
			LeaveCriticalSection(&cs);
			Sleep(1);
			SetEvent(evFin);
			break;
		}
		LeaveCriticalSection(&cs);
		Sleep(1);
	}
	return 0;
}


int main()
{
	evFin = CreateEvent(NULL, FALSE, FALSE, NULL);
	DWORD threadID;

	InitializeCriticalSection(&cs);

	CreateThread(NULL, 0, thProc1, NULL, 0, &threadID);
	CreateThread(NULL, 0, thProc2, NULL, 0, &threadID);

	WaitForMultipleObjects(1, &evFin, true, INFINITE);

	DeleteCriticalSection(&cs);

	system("pause");
    return 0;
}

