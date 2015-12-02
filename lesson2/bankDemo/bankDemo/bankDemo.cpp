// bankDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
using namespace std;

int money = 10000;
int time = 0;
HANDLE evFin[2];
CRITICAL_SECTION cs;

DWORD WINAPI draw(LPVOID) {
	while (true) {
		Sleep(500);
		EnterCriticalSection(&cs);
		if (money > 3000)
		{
			money -= 3000;
			cout << "you withdraw $3000, you have $" << money << " now.\n";
		}
		else {
			cout << "you don't have enough money.\n";
			LeaveCriticalSection(&cs);
			SetEvent(evFin[0]);
			break;
		}
		if (time >= 1000*12*5)
		{
			cout << "five years go.\n";
			LeaveCriticalSection(&cs);
			SetEvent(evFin[0]);
			break;
		}
		LeaveCriticalSection(&cs);
	}
	return 0;
}

DWORD WINAPI save(LPVOID) {
	while (true)
	{
		Sleep(1000);
		EnterCriticalSection(&cs);
		money += 10000;
		time += 1000;
		cout << "you save $10000, you have $" << money << " now.\n";
		if (time >= 1000 * 12 * 5) {
			cout << "five years go.\n";
			LeaveCriticalSection(&cs);
			SetEvent(evFin[1]);
			break;
		}
		LeaveCriticalSection(&cs);
	}
	return 0;
}

int main()
{
	evFin[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	evFin[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
	DWORD threadId;

	InitializeCriticalSection(&cs);

	CreateThread(NULL, 0, draw, NULL, 0, &threadId);
	CreateThread(NULL, 0, save, NULL, 0, &threadId);

	WaitForMultipleObjects(2, evFin, true, INFINITE);

	DeleteCriticalSection(&cs);

	system("pause");
    return 0;
}

