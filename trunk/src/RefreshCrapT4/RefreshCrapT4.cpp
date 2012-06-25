// RefreshCrapT4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdlib>
#include <Windows.h>
#include <vector>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>

struct ScanRange
{
	int lowBound;
	int highBound;
};

#define WPARAM_UPDATE_NOTIFY 0x3039
#define WPARAM_UPDATE_GOAHEAD 0x303A
#define WPARAM_UPDATE_CONFIRM 0x303D

#define SCAN_RANGES_SIZE 5

using namespace std;

typedef vector<HWND> HWNDVEC;

static TCHAR* TitleTimeStrings[] = { _T(",M1]"), _T(",M5]"), _T(",M15]"), _T(",M30]"), _T(",H1]"), _T(",H4]"), _T(",D1]"), _T(",W1]") ,_T(",MN]") };
static ScanRange LparamScanRanges[] = { { 0xC100, 0xC500 }, { 0xC000, 0xC100 }, { 0xC500, 0xD000 }, { 0x0000, 0xC000 }, { 0xD000, 0xFFFF } };

HWNDVEC _mt4s;
int _msg;

BOOL CALLBACK ExamineWindow(HWND wnd, LPARAM lparam)
{
	TCHAR buffer[128];
	GetWindowText(wnd, buffer, 128);
	int idx = 0;
	for (; idx < 9 && _tcsstr(buffer, TitleTimeStrings[idx]) == NULL; idx++) ;

	if (idx < 9)
		_mt4s.push_back(wnd);
	return TRUE;
}

bool NotifyCrapT4(HWND handle, int lparam) 
{
	if (WPARAM_UPDATE_GOAHEAD == SendMessage(handle, _msg, WPARAM_UPDATE_NOTIFY, lparam))
	{
		PostMessageA(handle, _msg, WPARAM_UPDATE_CONFIRM, lparam);
		Sleep(100);
		PostMessageA(handle, _msg, 2, 1); // simulate a tick

		return true;
	}

	return false;
}

vector<int>* ScanParams()
{
	vector<int>* params = (vector<int>*)0;
	for (HWNDVEC::iterator itr = _mt4s.begin(); itr != _mt4s.end(); itr++)
	{
		bool gotIt = false;
		for (int i = 0; !gotIt && i < SCAN_RANGES_SIZE; i++)
		{
			for (int j = LparamScanRanges[i].lowBound; !gotIt && j < LparamScanRanges[i].highBound; j++)
				if (WPARAM_UPDATE_GOAHEAD == SendMessage(*itr, _msg, WPARAM_UPDATE_NOTIFY, j))
				{
					if (!params)
						params = new vector<int>;
					(*params).push_back(j);
					gotIt = true;
				}
		}
		if (!gotIt)
			cerr << "Scan fails, please notify project owner when you see this message";
	}

	return params;
}

vector<int>* ReadSavedLparams(TCHAR* fileName)
{
	ifstream file(fileName, ios_base::in);
	if (file)
	{
		vector<int>* result = new vector<int>;
		int number;
		while (!file.eof())
		{
			string filler;
			file >> number >> filler;
			if (number > 0)
				(*result).push_back(number);
		}
		file.close();
		return result;
	}
	return (vector<int>*)0;
}

void SaveParams(_TCHAR* fileName, vector<int>* params)
{
	if (params)
	{
		ofstream file(fileName, ios_base::out);
		for (vector<int>::iterator itr = (*params).begin(); itr != (*params).end(); itr++)
			file << *itr << endl;
		file.close();
	}
}

TCHAR* GetLparamFileName()
{
	return _T("lparams.txt");
}

bool SendMessageToCrapT4(HWNDVEC handles, vector<int> *params)
{
	if (!params)
	{
		params = ScanParams();
		if (!params)
			return false;

		SaveParams(GetLparamFileName(), params);
	}

	bool atLeast1Success = false;
	for (HWNDVEC::iterator itr = _mt4s.begin(); itr != _mt4s.end(); itr++) {
		for (vector<int>::iterator p = (*params).begin(); p != (*params).end(); p++)
			atLeast1Success |= NotifyCrapT4(*itr, *p);
	}

	if (!atLeast1Success)
	{
		delete params;
		params = (vector<int>*)0;
		return SendMessageToCrapT4(handles, params);
	}

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	_msg = RegisterWindowMessageA("MetaTrader4_Internal_Message");
	EnumDesktopWindows(NULL, ExamineWindow, 0);
	vector<int>* params = ReadSavedLparams(GetLparamFileName());
	SendMessageToCrapT4(_mt4s, params);
	return 0;
}