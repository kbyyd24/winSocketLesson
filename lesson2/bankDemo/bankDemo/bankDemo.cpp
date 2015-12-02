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
HANDLE th[2];

DWORD WINAPI draw(LPVOID param) {
	while (true) {
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
		Sleep(500);
	}
	return 0;
}

DWORD WINAPI save(LPVOID param) {
	while (true) {
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
		Sleep(1000);
	}
	return 0;
}

DWORD WINAPI quitProc(LPVOID param) {
	char input;
	while (true)
	{
		cin >> input;
		if ('q' == input) {
			TerminateThread(th[0], 1);
			TerminateThread(th[1], 1);
			SetEvent(evFin[0]);
			SetEvent(evFin[1]);
			cout << "prcesses will shutdown\n";
			break;
		}
		else {
			cout << "error command!\n";
		}
	}
	return 0;
}

int main()
{
	evFin[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	evFin[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
	DWORD threadId, ctlId;
	HANDLE thCtl;

	InitializeCriticalSection(&cs);

	th[0] = CreateThread(NULL, 0, draw, NULL, 0, &threadId);
	th[1] = CreateThread(NULL, 0, save, NULL, 0, &threadId);
	thCtl = CreateThread(NULL, 0, quitProc, NULL, 0, &ctlId);

	WaitForMultipleObjects(2, evFin, true, INFINITE);
	TerminateThread(thCtl, 1);

	DeleteCriticalSection(&cs);

	system("pause");
    return 0;
}

