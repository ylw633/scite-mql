// RefreshCrapT4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdlib>
#include <Windows.h>
#include <vector>
#include <tchar.h>
#include <iostream>

using namespace std;

typedef vector<HWND> HWNDVEC;

HWNDVEC _mt4s;

BOOL CALLBACK ExamineWindow(HWND wnd, LPARAM lparam)
{
	TCHAR buffer[128];
	GetWindowText(wnd, buffer, 128);
	if (_tcsstr(buffer, _T("MetaTrader")) || _tcsstr(buffer, _T("MT4")))
	{
		//_tcprintf(_T("%d %s\n"), wnd, buffer);
		_mt4s.push_back(wnd);
	}
	return TRUE;
}

void SendMessageToCrapT4(HWNDVEC handles)
{
	int msg = RegisterWindowMessageA("MetaTrader4_Internal_Message");
	for (HWNDVEC::iterator itr = _mt4s.begin(); itr != _mt4s.end(); itr++) {
		int result = SendMessage(*itr, msg, 0x3039, 0xc413);
		//PostMessageA(*itr, msg, 0x0000303D, 0xC413);
		Sleep(100);
		PostMessageA(*itr, msg, 0x0000303D, 0xC413);
		Sleep(300);
		PostMessageA(*itr, msg, 2, 1);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	EnumDesktopWindows(NULL, ExamineWindow, 0);
	SendMessageToCrapT4(_mt4s);
	return 0;
}