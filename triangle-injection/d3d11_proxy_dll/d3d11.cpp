#pragma once
#include <Windows.h>
#include "debug.h"
#include <stdint.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <d3d11_4.h>
#include <shlwapi.h>
#include "hooking.h"

#pragma comment (lib, "Shlwapi.lib") //for PathRemoveFileSpecA
#pragma comment(lib, "d3dcompiler.lib")


typedef HRESULT(__stdcall* fn_D3D11CreateDeviceAndSwapChain)(
	IDXGIAdapter*,
	D3D_DRIVER_TYPE,
	HMODULE,
	UINT,
	const D3D_FEATURE_LEVEL*,
	UINT,
	UINT,
	const DXGI_SWAP_CHAIN_DESC*,
	IDXGISwapChain**,
	ID3D11Device**,
	D3D_FEATURE_LEVEL*,
	ID3D11DeviceContext**);

typedef HRESULT(__stdcall* fn_DXGISwapChain_Present)(IDXGISwapChain*, UINT, UINT);

IDXGISwapChain* swapChain = nullptr;
ID3D11Device5* device = nullptr;
ID3D11DeviceContext4* devCon = nullptr;
ID3D10Blob* vs_blob = nullptr;
ID3D11VertexShader* vs = nullptr;
ID3D10Blob* ps_blob = nullptr;
ID3D11PixelShader* ps = nullptr;
ID3D11Buffer* vertex_buffer = nullptr;
ID3D11InputLayout* vertLayout = nullptr;
ID3D11RasterizerState* SolidRasterState = nullptr;
ID3D11DepthStencilState* SolidDepthStencilState = nullptr;

HRESULT DXGISwapChain_Present_Hook(IDXGISwapChain* thisPtr, UINT SyncInterval, UINT Flags)
{
	devCon->VSSetShader(vs, 0, 0);
	devCon->PSSetShader(ps, 0, 0);
	devCon->IASetInputLayout(vertLayout);
	devCon->RSSetState(SolidRasterState);
	devCon->OMSetDepthStencilState(SolidDepthStencilState, 0);

	UINT stride = sizeof(float) * 6;
	UINT offset = 0;
	devCon->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devCon->Draw(3, 0);


	fn_DXGISwapChain_Present DXGISwapChain_Present_Orig;
	PopAddress(uint64_t(&DXGISwapChain_Present_Orig));

	HRESULT r = DXGISwapChain_Present_Orig(thisPtr, SyncInterval, Flags);
	return r;
}


void LoadShaders()
{
	{
		char filepath[512];
		HMODULE hModule = GetModuleHandle(NULL);
		GetModuleFileNameA(hModule, filepath, 512);
		PathRemoveFileSpecA(filepath);

		strcat_s(filepath, 512, "\\hook_content\\passthrough_vs.shader");

		wchar_t wPath[513];
		size_t outSize;

		mbstowcs_s(&outSize, &wPath[0], strlen(filepath) + 1, filepath, strlen(filepath));
		ID3D10Blob* compileErrors = nullptr;

		HRESULT err = D3DCompileFromFile(wPath, 0, 0, "main", "vs_5_0", 0, 0, &vs_blob, &compileErrors);
		if (compileErrors != nullptr && compileErrors)
		{
			ID3D10Blob* outErrorsDeref = compileErrors;
			OutputDebugStringA((char*)compileErrors->GetBufferPointer());
		}

		err = device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), NULL, &vs);
		check(err == S_OK);
	}
	{
		char filepath[512];
		HMODULE hModule = GetModuleHandle(NULL);
		GetModuleFileNameA(hModule, filepath, 512);
		PathRemoveFileSpecA(filepath);

		strcat_s(filepath, 512, "\\hook_content\\vertex_color_ps.shader");

		wchar_t wPath[513];
		size_t outSize;

		mbstowcs_s(&outSize, &wPath[0], strlen(filepath) + 1, filepath, strlen(filepath));
		ID3D10Blob* compileErrors;

		HRESULT err = D3DCompileFromFile(wPath, 0, 0, "main", "ps_5_0", 0, 0, &ps_blob, &compileErrors);
		if (compileErrors != nullptr && compileErrors)
		{
			ID3D10Blob* outErrorsDeref = compileErrors;
			OutputDebugStringA((char*)compileErrors->GetBufferPointer());
		}

		err = device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), NULL, &ps);
		check(err == S_OK);
	}
}


void CreateMesh()
{
	const float vertData[] =
	{
		-1, -1, 0.1,	1,0,0,
		1, 1, 0.1,	0,1,0,
		-1, 1, 0.1,	0,0,1
	};

	D3D11_BUFFER_DESC vertBufferDesc;
	ZeroMemory(&vertBufferDesc, sizeof(vertBufferDesc));
	vertBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertBufferDesc.ByteWidth = sizeof(float) * 6 * 3; //6 floats per vert, 3 verts
	vertBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertBufferDesc.CPUAccessFlags = 0;
	vertBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertBufferData;
	ZeroMemory(&vertBufferData, sizeof(vertBufferData));
	vertBufferData.pSysMem = vertData;

	HRESULT res = device->CreateBuffer(&vertBufferDesc, &vertBufferData, &vertex_buffer);
	check(res == S_OK);
}


void CreateInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertElements[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HRESULT err = device->CreateInputLayout(vertElements, _countof(vertElements), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &vertLayout);
	check(err == S_OK);
}

void CreateRasterizerAndDepthStates()
{
	D3D11_RASTERIZER_DESC soliddesc;
	ZeroMemory(&soliddesc, sizeof(D3D11_RASTERIZER_DESC));
	soliddesc.FillMode = D3D11_FILL_SOLID;
	soliddesc.CullMode = D3D11_CULL_NONE;
	HRESULT err = device->CreateRasterizerState(&soliddesc, &SolidRasterState);
	check(err == S_OK);

	D3D11_DEPTH_STENCIL_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	err = device->CreateDepthStencilState(&depthDesc, &SolidDepthStencilState);
	check(err == S_OK);
}

fn_D3D11CreateDeviceAndSwapChain LoadD3D11AndGetOriginalFuncPointer()
{
	char path[MAX_PATH];
	if (!GetSystemDirectoryA(path, MAX_PATH)) return nullptr;

	strcat_s(path, MAX_PATH * sizeof(char), "\\d3d11.dll");
	HMODULE d3d_dll = LoadLibraryA(path); 

	if (!d3d_dll)
	{
		MessageBox(NULL, TEXT("Could Not Locate Original D3D11 DLL"), TEXT("Darn"), 0);
		return nullptr;
	}

	return (fn_D3D11CreateDeviceAndSwapChain)GetProcAddress(d3d_dll, TEXT("D3D11CreateDeviceAndSwapChain"));
}

inline void** get_vtable_ptr(void* obj)
{
	return *reinterpret_cast<void***>(obj);
}

extern "C" HRESULT __stdcall D3D11CreateDeviceAndSwapChain(
	IDXGIAdapter * pAdapter,
	D3D_DRIVER_TYPE            DriverType,
	HMODULE                    Software,
	UINT                       Flags,
	const D3D_FEATURE_LEVEL * pFeatureLevels,
	UINT                       FeatureLevels,
	UINT                       SDKVersion,
	const DXGI_SWAP_CHAIN_DESC * pSwapChainDesc,
	IDXGISwapChain * *ppSwapChain,
	ID3D11Device * *ppDevice,
	D3D_FEATURE_LEVEL * pFeatureLevel,
	ID3D11DeviceContext * *ppImmediateContext
)
{
	//uncomment if you need to debug an issue in a project you aren't launching from VS
	//this gives you an easy way to make sure you can attach a debugger at the right time
	//MessageBox(NULL, TEXT("Calling D3D11CreateDeviceAndSwapChain"), TEXT("Ok"), 0);

	fn_D3D11CreateDeviceAndSwapChain D3D11CreateDeviceAndSwapChain_Orig = LoadD3D11AndGetOriginalFuncPointer();

	HRESULT res = D3D11CreateDeviceAndSwapChain_Orig(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);

	HRESULT hr = (*ppDevice)->QueryInterface(__uuidof(ID3D11Device5), (void**)&device);
	hr = (*ppImmediateContext)->QueryInterface(__uuidof(ID3D11DeviceContext), (void**)&devCon);

	LoadShaders();
	CreateMesh();
	CreateInputLayout();
	CreateRasterizerAndDepthStates();

	swapChain = *ppSwapChain;
	void** swapChainVTable = get_vtable_ptr(swapChain);
	
	InstallHook(swapChainVTable[8], DXGISwapChain_Present_Hook);
	//present is [8];

	return res;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		MessageBox(NULL, TEXT("Target app has loaded your proxy d3d11.dll and called DllMain. If you're launching Skyrim via steam, you need to dismiss this popup quickly, otherwise you get a load error"), TEXT("Success"), 0);
	}

	return true;
}
