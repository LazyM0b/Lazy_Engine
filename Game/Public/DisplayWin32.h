#pragma once

#include <windows.h>
#include <WinUser.h>

#include "iostream"

class DisplayWin32 {
public:
	DisplayWin32();
	DisplayWin32(int width, int height);
	HWND Init(HINSTANCE hInst, LPCWSTR appName);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
	
	unsigned int clientHeight;
	unsigned int clientWidth;
	HINSTANCE hInstance;
	HWND hWindow;
	WNDCLASSEX wc;
};