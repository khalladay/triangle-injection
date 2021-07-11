#pragma once
#include <string>
#include "common_header.h"

class SimpleApplication
{
public:
	static SimpleApplication* GetInstance();

	void Initialize(HWND wndHdl, HINSTANCE instance, const char* pathToExe);
	void Tick();
	void Draw();
	bool ShouldExit();

	uint32_t GetScreenW() const { return screenW; }
	uint32_t GetScreenH() const { return screenH; }

	std::string getExecutablePath() const;
	std::string GetAbsolutePathForContent(std::string filename);
	std::wstring GetAbsolutePathForContentW(std::string filename);

	HWND appWindowHandle;

private:
	SimpleApplication();
	~SimpleApplication();

	bool wantsExit;
	std::string pathToExe;

	uint32_t screenW;
	uint32_t screenH;

	class DX11RenderBackend* dx11;
	class SimpleView* view;
};

SimpleApplication& App();
