#include "common_header.h"
#include <stdio.h>
#include "win_init.h"
#include <shlwapi.h>
#pragma comment (lib, "Shlwapi.lib")

void mainLoop();
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE pInstance, LPSTR cmdLine, int showCode)
{
	HWND wndHdl = os::makeWindow(Instance, "Spinning Cube", 1024, 768);

	//this pointer is going to be passed to the app and left live for duration of project
	//this way the sketchbook class doesn't need windows specific code for getting the path
	char path[512];
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileNameA(hModule, path, 512);
	PathRemoveFileSpecA(path);

	App().Initialize(wndHdl, Instance, path);

	mainLoop();

	return 0;
}


void mainLoop()
{
	while (1)
	{
		os::handleEvents();
		App().Tick();
		App().Draw();
	}
}