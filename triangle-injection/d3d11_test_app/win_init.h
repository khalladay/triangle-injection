#pragma once

#include "delegate.h"
#include <Windows.h>

namespace os
{
	struct AppInfo
	{
		HWND wndHdl;
		HINSTANCE instance;
		double initialMS = 0.0;
	};

	extern AppInfo GAppInfo;

	extern Delegate<void, HWND, UINT, WPARAM, LPARAM> wndProcDelegate;

	double getMilliseconds();

	void handleEvents();
	HWND makeWindow(HINSTANCE Instance, const char* title, unsigned int width, unsigned int height);
}