/*===========================================================================
 * CherryBuilder - The Productivity Extension for C++Builder®
 *---------------------------------------------------------------------------
 * Copyright (C) 2017 Florian Koch <flko@mail.de>
 * All Rights Reserved
 *---------------------------------------------------------------------------
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 *===========================================================================
 */
 
#include <vcl.h>
#pragma hdrstop

#include "cherrybuilder_winmode.h"

#include <StrUtils.hpp>

#include <windows.h>
#include <vector>
#include <math.h>
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{

std::vector<TWinComb> WinMode::WinHandles;
std::vector<TWinComb> WinMode::ChildWinHandles;
//---------------------------------------------------------------------------

void WinMode::RefreshWinHandles()
{
	WinHandles.clear();
	ChildWinHandles.clear();

	EnumWindows(EnumWindowsProc, NULL);

	for(unsigned int i=0; i<WinHandles.size(); i++)
    {
		EnumChildWindows(
            WinHandles.at(i).Handle, 
            EnumChildWindowsProc, 
            reinterpret_cast<long>(WinHandles.at(i).Handle)
            );
    }

	WinHandles.insert(WinHandles.end(), ChildWinHandles.begin(), ChildWinHandles.end());
}
//---------------------------------------------------------------------------

TPoint WinMode::GetAbsolutePos(TCoordMode Mode, HWND WinHandle, int x, int y)
{
	TPoint TempPos;
	tagRECT WinRect;
	tagPOINT TempPoint;

	TempPos.x = 0;
	TempPos.y = 0;

	switch(Mode)
	{
		case cmScreen:
			TempPos.x = x;
			TempPos.y = y;
		break;

		case cmWindow:
			GetWindowRect(WinHandle, &WinRect);
			TempPos.x = WinRect.left + x;
			TempPos.y = WinRect.top + y;
		break;

		case cmClient:
			GetClientRect(WinHandle, &WinRect);
			TempPoint.x = WinRect.left;
			TempPoint.y = WinRect.top;
			ClientToScreen(WinHandle, &TempPoint);//?
			TempPos.x = TempPoint.x + x;
			TempPos.y = TempPoint.y + y;
		break;
	}

	return TempPos;
}
//---------------------------------------------------------------------------

BOOL CALLBACK WinMode::EnumWindowsProc(HWND hWnd, LPARAM lparam)
{
	wchar_t pcWinTitle[256];
	wchar_t pcWinClassName[256];
	TWinComb WinHandle;
	long pcID;

	GetWindowText(hWnd, pcWinTitle, 255);
	GetClassName(hWnd, pcWinClassName, 255);
	pcID = GetWindowLongPtr(hWnd, GWLP_ID);

	WinHandle.Handle = hWnd;
	WinHandle.ParentHandle = NULL;
	WinHandle.Name = String(pcWinTitle);
	WinHandle.ClassName = String(pcWinClassName);
	WinHandle.ID = pcID;

	WinHandles.push_back(WinHandle);

	return TRUE;
}
//---------------------------------------------------------------------------

BOOL CALLBACK WinMode::EnumChildWindowsProc(HWND hWnd, LPARAM lParam)
{
	wchar_t pcWinTitle[256];
	wchar_t pcWinClassName[256];
	TWinComb ChildWinHandle;
	long pcID;

	GetWindowText(hWnd, pcWinTitle, 255);
	GetClassName(hWnd, pcWinClassName, 255);
	pcID = GetWindowLongPtr(hWnd, GWLP_ID);

	ChildWinHandle.Handle = hWnd;
	ChildWinHandle.ParentHandle = reinterpret_cast<HWND>(lParam);
	ChildWinHandle.Name = String(pcWinTitle);
	ChildWinHandle.ClassName = String(pcWinClassName);
	ChildWinHandle.ID = pcID;

	ChildWinHandles.push_back(ChildWinHandle);

	return TRUE;
}
//---------------------------------------------------------------------------

HWND WinMode::WinGetHandleByTitle(String WndTitle, TWinMatchMode Mode)
{
	HWND Hndl = NULL;
	bool Continue = true;

	RefreshWinHandles();

	for(unsigned int i=0; i<WinHandles.size() && Continue; i++)
	{
		switch(Mode)
		{
			case wmmComplete:
            
				if(WndTitle == WinHandles.at(i).Name)
				{
					Hndl = WinHandles.at(i).Handle;
					Continue = false;
				}
                
			break;

			case wmmBegin:
            
				if(StartsStr(WndTitle, WinHandles.at(i).Name))
				{
					Hndl = WinHandles.at(i).Handle;
					Continue = false;
				}
                
			break;

			case wmmEnd:
            
				if(EndsStr(WndTitle, WinHandles.at(i).Name))
				{
					Hndl = WinHandles.at(i).Handle;
					Continue = false;
				}
                
			break;

			case wmmContain:
            
				if(ContainsStr(WndTitle, WinHandles.at(i).Name))
				{
					Hndl = WinHandles.at(i).Handle;
					Continue = false;
				}
                
			break;
		}
	}

	return Hndl;
}
//---------------------------------------------------------------------------

HWND WinMode::WinGetHandleByClassName(String WndClassName, TWinMatchMode Mode)
{
	HWND Hndl = NULL;

	RefreshWinHandles();

	for(unsigned int i=0; i<WinHandles.size(); i++)
	{
		switch(Mode)
		{
			case wmmComplete:
				if(WndClassName == WinHandles.at(i).ClassName)
					Hndl = WinHandles.at(i).Handle;
			break;

			case wmmBegin:
				if(StartsStr(WndClassName, WinHandles.at(i).ClassName))
					Hndl = WinHandles.at(i).Handle;
			break;

			case wmmEnd:
				if(EndsStr(WndClassName, WinHandles.at(i).ClassName))
					Hndl = WinHandles.at(i).Handle;
			break;

			case wmmContain:
				if(ContainsStr(WndClassName, WinHandles.at(i).ClassName))
					Hndl = WinHandles.at(i).Handle;
			break;
		}
	}

	return Hndl;
}
//---------------------------------------------------------------------------

void WinMode::WinShow(HWND WinHandle)
{
	ShowWindow(WinHandle, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void WinMode::WinShowMaximized(HWND WinHandle)
{
	ShowWindow(WinHandle, SW_SHOWMAXIMIZED);
}
//---------------------------------------------------------------------------

void WinMode::WinHide(HWND WinHandle)
{
	ShowWindow(WinHandle, SW_HIDE);
}
//---------------------------------------------------------------------------

void WinMode::WinActivate(HWND WinHandle)
{
	SetForegroundWindow(WinHandle);
}
//---------------------------------------------------------------------------

bool WinMode::WinIsVisible(HWND WinHandle)
{
	return IsWindowVisible(WinHandle);
}
//---------------------------------------------------------------------------

void WinMode::MoveMouse(int x, int y, int speed, TCoordMode Mode, HWND WinHandle)
{
	POINT AbsoluteCursorPos = GetAbsolutePos(Mode, WinHandle, x, y);
	POINT mouse;
	double mx, my, nx, ny, len;

    //Cursor darf sich auf ganzem Bildschirm bewegen
	ClipCursor(NULL);

	GetCursorPos(&mouse);
	mx = mouse.x;
	my = mouse.y;

	while(ceil(mx) != AbsoluteCursorPos.x || ceil(my) != AbsoluteCursorPos.y)
	{
		nx = AbsoluteCursorPos.x-mx;
		ny = AbsoluteCursorPos.y-my;
		len = sqrt(nx*nx + ny*ny);

		if(len <= 1)
		{
			mx=AbsoluteCursorPos.x;
			my=AbsoluteCursorPos.y;
		}
		else
		{
			nx /= (len*0.5);
			ny /= (len*0.5);
			mx += nx;
			my += ny;
		}

		SetCursorPos(ceil(mx), ceil(my));
		Sleep(speed);
	}
}
//---------------------------------------------------------------------------

void WinMode::MouseClick(TMouseButton Button, int NumberOfClicks)
{
	for(int i=0; i<NumberOfClicks; i++)
	{
		switch(Button)
		{
			case mbLeft:
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;

			case mbMiddle:
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);

			break;

			case mbRight:
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		}
	}
}
//---------------------------------------------------------------------------

void WinMode::WinMove(HWND WinHandle, int x, int y, int cx, int cy)
{
	SetWindowPos(WinHandle, HWND_TOPMOST, x, y, cx, cy, SWP_NOACTIVATE);
}
//---------------------------------------------------------------------------

void WinMode::ControlInsertText(String SendText)
{
    AnsiString text = AnsiString(SendText);
	int i, j;
	unsigned char ch;
	AnsiString str;

	i = 1;

	while(i<=text.Length())
	{
		ch = text[i];

		if(IsDBCSLeadByte(ch))
		{
			i++;
			str = MAKEWORD(text[i], ch);
			keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), 0, 0);
			j=1;
			while(j<=str.Length())
			{
				keybd_event(96+StrToInt(str[j]), MapVirtualKey(96+StrToInt(str[j]), 0), 0, 0);
			   keybd_event(96+StrToInt(str[j]), MapVirtualKey(96+StrToInt(str[j]), 0), 2, 0);
			   j++;
			}
			keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), KEYEVENTF_KEYUP, 0);
		}
		else
		{
			keybd_event(VkKeyScan(text[i]),0,0,0);
			keybd_event(VkKeyScan(text[i]),0,2,0);
		}
		i++;
    }
}
//---------------------------------------------------------------------------

void WinMode::ControlClick(HWND WinHandle, String ControlTitle)
{
	for(unsigned int i=0; i<WinHandles.size(); i++)
	{
		if(	(WinHandles.at(i).ParentHandle == WinHandle)
			&&	(WinHandles.at(i).Name == ControlTitle)	)
		{
			SendMessage(WinHandles.at(i).Handle, BM_CLICK, 0, 0);

			break;
        }
    }
}
//---------------------------------------------------------------------------

void WinMode::GlobalShowCursor(bool bShow)
{
	TRect r;

	if(!bShow)
	{
		r.Top = 0;
		r.Left = GetSystemMetrics(SM_CXSCREEN) + GetSystemMetrics(SM_CXCURSOR);
		r.Right = r.Left;
		r.Bottom = 0;

		ClipCursor(&r);
		SetCursorPos(0, 0);
	}
	else
	{
		ClipCursor(NULL);
		SetCursorPos(GetSystemMetrics(SM_CXSCREEN) / 2, GetSystemMetrics(SM_CYSCREEN) / 2);
	}
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder

