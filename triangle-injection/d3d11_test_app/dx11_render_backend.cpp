#include "dx11_render_backend.h"
#pragma comment(lib, "d3d11.lib")


DX11RenderBackend& DX() { return *DX11RenderBackend::getInstance(); }

DX11RenderBackend* DX11RenderBackend::getInstance()
{
	static DX11RenderBackend* instance = nullptr;
	if (!instance)
	{
		instance = new DX11RenderBackend();
	}
	return instance;
}

DX11RenderBackend::DX11RenderBackend()
{
	DXGI_MODE_DESC backBufferDesc;
	ZeroMemory(&backBufferDesc, sizeof(DXGI_MODE_DESC));

	backBufferDesc.Width = App().GetScreenW();
	backBufferDesc.Height = App().GetScreenH();
	backBufferDesc.RefreshRate.Numerator = 60;
	backBufferDesc.RefreshRate.Denominator = 1;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	backBufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	backBufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.SampleDesc.Count = 8;
	swapChainDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = App().appWindowHandle;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; 

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = App().GetScreenW();
	depthStencilDesc.Height = App().GetScreenH();
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 8;
	depthStencilDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

#ifdef _DEBUG
	bool isDebugBuild = false;
#else
	bool isDebugBuild = false;
#endif

	ID3D11Device* tempDevice;
	ID3D11DeviceContext* tempContext;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,  //use default adapter
		D3D_DRIVER_TYPE_HARDWARE, //use the gpu for direct3d
		NULL, //don't use software rasterizing
		isDebugBuild ? D3D11_CREATE_DEVICE_DEBUG : NULL, //no flags
		NULL, //use the highest feature level available
		NULL, //number of elements in the previous feature level array
		D3D11_SDK_VERSION,  //the version of the sdk to use
		&swapChainDesc,
		&swapChain,
		&tempDevice,
		NULL, //pointer to get the highest feature level available
		&tempContext);

	checkf(!FAILED(hr), "Failed to create swapchain and device");

	//we need a ID3D11Device5 interface so we can use fences to synchronize with compute shaders
	hr = tempDevice->QueryInterface(__uuidof(ID3D11Device5), (void**)&device);
	checkf(!FAILED(hr), "Failed to cast device to ID3D11Device5");
	check(device);

	hr = tempContext->QueryInterface(__uuidof(ID3D11DeviceContext4), (void**)&devCon);
	checkf(!FAILED(hr), "Failed to cast device to ID3D11DeviceContext4");
	check(devCon);

	ID3D11Texture2D* backBuffer;
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	checkf(!FAILED(hr), "Failed to get back buffer");

	hr = device->CreateRenderTargetView(backBuffer, NULL, &rtView);
	checkf(!FAILED(hr), "Failed to create rtv for back buffer");
	backBuffer->Release();

	hr = device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	checkf(!FAILED(hr), "Failed to create depth buffer");

	hr = device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);
	checkf(!FAILED(hr), "Failed to create depth view");


	SetDefaultRenderTargets();
}

DX11RenderBackend::~DX11RenderBackend()
{
	swapChain->Release();
	devCon->Release();
	device->Release();
}

void DX11RenderBackend::SetDefaultRenderTargets()
{
	ID3D11RenderTargetView* rtvs[] = { rtView} ;
	devCon->OMSetRenderTargets(_countof(rtvs), rtvs, depthStencilView);
}

ID3D11Buffer* DX11RenderBackend::CreateConstantBuffer(uint32_t size, D3D11_USAGE usage, void* initData)
{
	ID3D11Buffer* outBuffer;

	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(D3D11_BUFFER_DESC));
	cbd.Usage = usage;

	cbd.ByteWidth = size;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
	cbd.MiscFlags = 0;
	if (initData)
	{
		D3D11_SUBRESOURCE_DATA iData;
		iData.pSysMem = initData;
		HRESULT err = device->CreateBuffer(&cbd, &iData, &outBuffer);
		check(err == S_OK);
	}
	else
	{
		HRESULT err = device->CreateBuffer(&cbd, NULL, &outBuffer);
		check(err == S_OK);
	}

	return outBuffer;
}

ID3D11Buffer* DX11RenderBackend::CreateIndexBuffer(uint32_t numVerts, const uint32_t* indices)
{
	ID3D11Buffer* outBuff;

	D3D11_BUFFER_DESC vertBufferDesc;
	ZeroMemory(&vertBufferDesc, sizeof(vertBufferDesc));
	vertBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertBufferDesc.ByteWidth = numVerts * sizeof(uint32_t);
	vertBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	vertBufferDesc.CPUAccessFlags = 0;
	vertBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertBufferData;
	ZeroMemory(&vertBufferData, sizeof(vertBufferData));

	vertBufferData.pSysMem = indices;

	HRESULT err = DX().device->CreateBuffer(&vertBufferDesc, &vertBufferData, &outBuff);
	check(err == S_OK);

	return outBuff;

}

ID3D11Buffer* DX11RenderBackend::CreateVertexBuffer(uint32_t numVerts, uint32_t vertSize, const float* vertData, const D3D11_INPUT_ELEMENT_DESC* inputLayout)
{
	ID3D11Buffer* outBuff;

	D3D11_BUFFER_DESC vertBufferDesc;
	ZeroMemory(&vertBufferDesc, sizeof(vertBufferDesc));
	vertBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertBufferDesc.ByteWidth = vertSize * numVerts;
	vertBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertBufferDesc.CPUAccessFlags = 0;
	vertBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertBufferData;
	ZeroMemory(&vertBufferData, sizeof(vertBufferData));

	vertBufferData.pSysMem = vertData;

	HRESULT err = DX().device->CreateBuffer(&vertBufferDesc, &vertBufferData, &outBuff);
	check(err == S_OK);

	return outBuff;

}
