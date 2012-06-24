// RefreshCrapT4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdlib>
#include <Windows.h>
#include <vector>
#include <tchar.h>
#include <iostream>

#define BRUTE_FORCE_LPARAM_LBOUND 0xC100
#define BRUTE_FORCE_LPARAM_UBOUND 0xC500

using namespace std;

typedef vector<HWND> HWNDVEC;

static TCHAR* TitleTimeStrings[] = { _T(",M1]"), _T(",M5]"), _T(",M15]"), _T(",M30]"), _T(",H1]"), _T(",H4]"), _T(",D1]"), _T(",W1]") ,_T(",MN]") };

HWNDVEC _mt4s;
int _presetLparam = 0;

BOOL CALLBACK ExamineWindow(HWND wnd, LPARAM lparam)
{
	TCHAR buffer[128];
	GetWindowText(wnd, buffer, 128);
	int idx = 0;
	for (; idx < 9 && _tcsstr(buffer, TitleTimeStrings[idx]) == NULL; idx++) ;

	if (idx < 9)
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
		int lparam = _presetLparam == 0 ? BRUTE_FORCE_LPARAM_LBOUND : _presetLparam; // there should be a better way to do this..
		int ubound = _presetLparam == 0 ? BRUTE_FORCE_LPARAM_UBOUND : _presetLparam;
		for (; lparam <= ubound; lparam++) 
		{
			int result = SendMessage(*itr, msg, 0x3039, lparam);
			if (result == 0x303A)
			{
				PostMessageA(*itr, msg, 0x0000303D, lparam);
				Sleep(100);
				PostMessageA(*itr, msg, 0x0000303D, lparam);
				Sleep(300);
				PostMessageA(*itr, msg, 2, 1);
				break;
			}
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc > 1)
		_presetLparam = _ttoi(argv[1]);
	EnumDesktopWindows(NULL, ExamineWindow, 0);
	SendMessageToCrapT4(_mt4s);
	return 0;
}