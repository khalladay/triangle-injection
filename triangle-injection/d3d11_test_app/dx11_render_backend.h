#pragma once
#include <d3d11_4.h>
#include <d3d10.h>
#include "common_header.h"
class DX11RenderBackend
{
public:
	static DX11RenderBackend* getInstance();

	IDXGISwapChain* swapChain = nullptr;
	ID3D11Device5* device = nullptr;
	ID3D11DeviceContext4* devCon = nullptr;
	ID3D11RenderTargetView* rtView = nullptr;

	ID3D11DepthStencilView* depthStencilView = nullptr;
	ID3D11Texture2D* depthStencilBuffer = nullptr;


	ID3D11Buffer* CreateConstantBuffer(uint32_t bufferSize, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, void* initData = nullptr);

	ID3D11Buffer* CreateVertexBuffer(uint32_t numVerts, uint32_t vertSize, const float* vertData, const D3D11_INPUT_ELEMENT_DESC* inputLayout);
	ID3D11Buffer* CreateIndexBuffer(uint32_t numVerts, const uint32_t* indices);


	void SetDefaultRenderTargets();
private:
	DX11RenderBackend();
	~DX11RenderBackend();

};

DX11RenderBackend& DX();
