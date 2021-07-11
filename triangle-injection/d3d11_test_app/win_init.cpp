#include "win_init.h"
#include "common_header.h"
#include "delegate.h"

#include <Windows.h>

#if _DEBUG
#include <stdio.h>
#include <consoleapi.h> //for console
#include <processthreadsapi.h> //for getcurrentprocessid
#endif



namespace os
{
	Delegate<void, HWND, UINT, WPARAM, LPARAM> wndProcDelegate;
	AppInfo GAppInfo;

	LRESULT CALLBACK defaultWndFunc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		os::wndProcDelegate.broadcast(window, message, wParam, lParam);
		LRESULT result = 0;
		switch (message)
		{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}break;
		default:
		{
			result = DefWindowProc(window, message, wParam, lParam);
		}break;
		}

		return result;
	}


	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);

	double getMilliseconds()
	{
		if (s_use_qpc) {
			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			return (1000LL * now.QuadPart) / (double)s_frequency.QuadPart - GAppInfo.initialMS;
		}
		else {
			return GetTickCount64() - GAppInfo.initialMS;
		}
	}

	void handleEvents()
	{
		MSG message;
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	HWND makeWindow(HINSTANCE Instance, const char* title, unsigned int width, unsigned int height)
	{
#if _DEBUG
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		FILE* pCout;
		freopen_s(&pCout, "conout$", "w", stdout);
		freopen_s(&pCout, "conin$", "w", stdin);
		freopen_s(&pCout, "conout$", "w", stderr);

		fclose(pCout);
#endif

		WNDCLASS windowClass = {};
		windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = defaultWndFunc;
		windowClass.hInstance = Instance;
		windowClass.lpszClassName = title;

		RECT windowRect;
		windowRect.top = 0;
		windowRect.left = 0;
		windowRect.right = width;
		windowRect.bottom = height;

		AdjustWindowRect(&windowRect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, FALSE);

		RegisterClass(&windowClass);
		HWND wndHdl = CreateWindowEx(0,
			windowClass.lpszClassName,
			title,
			WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, //can't be resized or maximized
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			(int)windowRect.right - windowRect.left,
			(int)windowRect.bottom - windowRect.top,
			0,
			0,
			Instance,
			0);

		GAppInfo.instance = Instance;
		GAppInfo.wndHdl = wndHdl;
		GAppInfo.initialMS = getMilliseconds();

		return wndHdl;
	}

}