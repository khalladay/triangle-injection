#include "simple_view.h"
#include <string>
#include "common_header.h"
#include "dX11_render_backend.h"
#include <vector>

D3D11_INPUT_ELEMENT_DESC vertElements[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const float cubeVerts[] =
{
	-0.5, -0.5, -0.5,	0.5,0.25,0,
	0.5, -0.5, -0.5,	0.5,1,0.5,
	0.5, 0.5, -0.5,		0.25,0.5,1,
	-0.5, 0.5,-0.5,		0.8,1,0,

	-0.5, -0.5, 0.5,	1,0,0.5,
	0.5, -0.5, 0.5,		0.5,0.24,0.5,
	0.5, 0.5, 0.5,		0.3,0.3,1,
	-0.5, 0.5, 0.5,		1,0.3,0
};

const uint32_t cubeIndices[] =
{
	0, 2, 1, //back face 
	2, 0, 3,

	4, 5, 6, //front face
	6, 7, 4,

	2,3,6, //top face
	6,3,7,

	0,1,4, //bottom face
	4,1,5,

	1,2,5, //right face
	5,2,6,

	0,4,3, //left face
	4,7,3
};

const uint32_t edgeIndices[] =
{
	0,1,
	1,2,
	2,3,
	3,0,
	4,5,
	5,6,
	6,7,
	7,4,
	0,4,
	1,5,
	2,6,
	3,7
};


void SimpleView::Setup()
{
	cubeVB = DX().CreateVertexBuffer(8, sizeof(float) * 6, cubeVerts, vertElements);
	cubeTriangleIB = DX().CreateIndexBuffer(_countof(cubeIndices), cubeIndices);
	cubeEdgesIB = DX().CreateIndexBuffer(_countof(edgeIndices), edgeIndices);

	float aspect = App().GetScreenW() / (float)App().GetScreenH();
	DirectX::XMMATRIX mat = DirectX::XMMatrixOrthographicLH((float)App().GetScreenW(), (float)App().GetScreenH(), 0.01f, 1000.0f); 
	DirectX::XMStoreFloat4x4(&projectionMatrix, mat);

	ID3D10Blob* vsByteCode = cubeVS.GetBytecode();
	HRESULT err = DX().device->CreateInputLayout(vertElements, _countof(vertElements), vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize(), &vertLayout);
	check(err == S_OK);

	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	err = DX().device->CreateRasterizerState(&wfdesc, &WireFrameRasterState);
	check(err == S_OK);

	D3D11_RASTERIZER_DESC soliddesc;
	ZeroMemory(&soliddesc, sizeof(D3D11_RASTERIZER_DESC));
	soliddesc.FillMode = D3D11_FILL_SOLID;
	soliddesc.CullMode = D3D11_CULL_NONE;
	err = DX().device->CreateRasterizerState(&soliddesc, &SolidRasterState);
	check(err == S_OK);

	D3D11_DEPTH_STENCIL_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	err = DX().device->CreateDepthStencilState(&depthDesc, &WireDepthStencilState);
	check(err == S_OK);

	D3D11_DEPTH_STENCIL_DESC soliddepthDesc;
	ZeroMemory(&soliddepthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	soliddepthDesc.DepthEnable = true;
	soliddepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	soliddepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	err = DX().device->CreateDepthStencilState(&soliddepthDesc, &SolidDepthStencilState);
	check(err == S_OK);


}

void SimpleView::Draw()
{
	auto* dx11 = DX11RenderBackend::getInstance();
	dx11->SetDefaultRenderTargets();

	float bgColor[4] = { 0.7f, 0.45f, 0.45f, 0.0f };
	float noColor[4] = { 0,0,0,0 };
	dx11->devCon->ClearRenderTargetView(dx11->rtView, bgColor);
	dx11->devCon->ClearDepthStencilView(dx11->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(App().GetScreenW());
	viewport.Height = static_cast<float>(App().GetScreenH());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	dx11->devCon->RSSetViewports(1, &viewport);

	cubeVS.SetParameters({ cubeTformMatrix,projectionMatrix,0.0 });

	auto* devCon = DX().devCon;

	UINT stride = sizeof(float) * 6;
	UINT offset = 0;
 
	cubeVS.Use(devCon); 
	trianglePS.Use(devCon);
	devCon->RSSetState(SolidRasterState);
	devCon->OMSetDepthStencilState(SolidDepthStencilState, 0);

	devCon->IASetVertexBuffers(0, 1, &cubeVB, &stride, &offset);
	devCon->IASetIndexBuffer(cubeTriangleIB, DXGI_FORMAT_R32_UINT, 0);
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devCon->IASetInputLayout(vertLayout);
	devCon->DrawIndexed(_countof(cubeIndices), 0, 0);

	cubeVS.SetParameters({ cubeTformMatrix,projectionMatrix,0.0});
	cubeVS.Use(devCon);
	edgePS.SetParameters({ {1.0f, 1.0f, 1.0f, 1.0f} });
	edgePS.Use(devCon);
	devCon->OMSetDepthStencilState(WireDepthStencilState, 0);

	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	devCon->IASetIndexBuffer(cubeEdgesIB, DXGI_FORMAT_R32_UINT, 0);

	devCon->RSSetState(WireFrameRasterState);
	devCon->DrawIndexed(_countof(edgeIndices), 0, 0);

	HRESULT err = dx11->swapChain->Present(0, 0);
	check(err == S_OK);


}

void SimpleView::Tick(float deltaTime)
{
	static float t = 0.0f;
	t += deltaTime * 0.001f;
	DirectX::XMFLOAT3 axis =  { 0.57735f, 0.57735f, 0.57735f };
	DirectX::XMVECTOR axisVec = XMLoadFloat3(&axis);

	DirectX::XMMATRIX newScale = DirectX::XMMatrixScaling(200.0,200.0,200.0);
	DirectX::XMMATRIX newTranslation = DirectX::XMMatrixTranslation(0.0,0.0,300.0);

	DirectX::XMMATRIX newRot = DirectX::XMMatrixRotationAxis(axisVec, t);

	DirectX::XMMATRIX newTform = DirectX::XMMatrixMultiply(newScale, DirectX::XMMatrixMultiply(newRot, newTranslation));
	DirectX::XMStoreFloat4x4(&cubeTformMatrix, newTform);
}

void SimpleView::Teardown()
{
	cubeVB->Release();
	cubeTriangleIB->Release();
	cubeEdgesIB->Release();
}

