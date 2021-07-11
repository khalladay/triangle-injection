#include "win_init.h"
#include "dx11_render_backend.h"
#include "simple_view.h"
#include "common_header.h"

SimpleApplication& App() { return *SimpleApplication::GetInstance(); }

SimpleApplication* SimpleApplication::GetInstance()
{
	static SimpleApplication* instance = nullptr;
	if (!instance)
	{
		instance = new SimpleApplication();
	}

	return instance;
}

SimpleApplication::SimpleApplication()
{
}

SimpleApplication::~SimpleApplication()
{
}


std::string SimpleApplication::getExecutablePath() const
{
	checkf(!pathToExe.empty(), "Path to exe was not set");
	return pathToExe;
}

void SimpleApplication::Initialize(HWND wndHdl, HINSTANCE instance, const char* pathToApp)
{
	appWindowHandle = wndHdl;
	checkf(pathToApp, "Attemtping to initialize ScenarioBookApp with a null executable path");

	pathToExe = pathToApp;

	RECT rect;
	GetClientRect(wndHdl, &rect);
	screenW = rect.right;
	screenH = rect.bottom;

	view = new SimpleView();
	view->Setup();
}

void SimpleApplication::Tick()
{
	static double lastTime = 0.0f;
	double currentTime = os::getMilliseconds();

	double delta = lastTime == 0.0f ? 0.16 : currentTime - lastTime;
	lastTime = currentTime;
	view->Tick((float)delta);

}

bool SimpleApplication::ShouldExit()
{
	return wantsExit;
}

void SimpleApplication::Draw()
{
	if (view != nullptr)
	{
		view->Draw();
	}

}

std::string SimpleApplication::GetAbsolutePathForContent(std::string filename)
{
	std::string path = App().getExecutablePath();
	path += PATH_SEPARATOR;
	path += "content";
	path += PATH_SEPARATOR;
	path += filename;

	auto FileExists = [](const char* path)
	{
		WIN32_FIND_DATAA FindFileData;
		HANDLE handle = FindFirstFileA(path, &FindFileData);
		bool found = handle != INVALID_HANDLE_VALUE;
		if (found) FindClose(handle);
		return found;
	};

	checkf(FileExists(path.c_str()), "File %s does not exist\n", path.c_str());

	char buffer[512];
	_fullpath(buffer, path.c_str(), 512);
	path = buffer;
	return path;
}

std::wstring SimpleApplication::GetAbsolutePathForContentW(std::string filename)
{
	std::string path = GetAbsolutePathForContent(filename);

	std::wstring output(path.size() + 1, L' ');
	size_t outSize;
	mbstowcs_s(&outSize, &output[0], path.size() + 1, path.c_str(), path.size());

	return output;
}