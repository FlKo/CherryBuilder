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
 
#ifndef cherrybuilder_winmodeH
#define cherrybuilder_winmodeH

#include <vector>
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

struct TWinComb
{
	HWND Handle;
	HWND ParentHandle;
	String Name;
	String ClassName;
	long ID;
};
//---------------------------------------------------------------------------

enum TWinMatchMode
{
	wmmComplete = 0,
	wmmBegin,
	wmmEnd,
	wmmContain
};
//---------------------------------------------------------------------------

enum TCoordMode
{
	cmScreen = 0,
	cmWindow,
	cmClient
};
//---------------------------------------------------------------------------

class WinMode
{
public:
	static HWND WinGetHandleByTitle(String Title, TWinMatchMode Mode);
	static HWND WinGetHandleByClassName(String WndClassName, TWinMatchMode Mode);
	static void WinShow(HWND WinHandle);
	static void WinShowMaximized(HWND WinHandle);
	static void WinHide(HWND WinHandle);
	static void WinActivate(HWND WinHandle);
	static bool WinIsVisible(HWND WinHandle);
	static void MoveMouse(int x, int y, int speed, TCoordMode Mode, HWND WinHandle);
	static void MouseClick(TMouseButton Button, int NumberOfClicks);
	static void WinMove(HWND WinHandle, int x, int y, int cx, int cy);
	static void ControlInsertText(String SendText);
	static void ControlClick(HWND WinHandle, String ControlTitle);
	static void GlobalShowCursor(bool bShow);
    
private:
	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lparam);
	static BOOL CALLBACK EnumChildWindowsProc(HWND hWnd, LPARAM lParam);

	static void RefreshWinHandles();
	static TPoint GetAbsolutePos(TCoordMode Mode, HWND WinHandle, int x, int y);
    
	static std::vector<TWinComb> WinHandles;
	static std::vector<TWinComb> ChildWinHandles;

};

} // namespace Cherrybuilder

#endif
