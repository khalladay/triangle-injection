#pragma once
#include "dx11_shader.h"

class CubeVS : public Shader<CubeVS, ID3D11VertexShader>
{
public:
	_declspec(align(16)) struct ConstantBuffer
	{
		_declspec(align(16)) DirectX::XMFLOAT4X4 LocalToWorld;
		_declspec(align(16)) DirectX::XMFLOAT4X4 WorldToClip;
		_declspec(align(16)) float DepthAdjust;
	};

	void BindResources()
	{
		BindConstantBuffer(DX().devCon, _paramBuffer);
	}
 
	void SetParameters(ConstantBuffer params)
	{
		if (!_paramBuffer)
		{
			_paramBuffer = DX().CreateConstantBuffer(sizeof(ConstantBuffer));
		}

		_cachedCB = params;

		DX().devCon->UpdateSubresource(_paramBuffer, 0, nullptr, &_cachedCB, 0, 0);
	}
 
	static std::string GetFilePath() { return App().GetAbsolutePathForContent("cube_vs.shader"); }
	static std::string GetEntryPoint() { return "main"; }

private:
	ConstantBuffer _cachedCB;
	ID3D11Buffer* _paramBuffer;

};


class TriCubePS : public Shader<TriCubePS, ID3D11PixelShader>
{
public:
	void BindResources() {}
	static std::string GetFilePath() { return App().GetAbsolutePathForContent("vertex_color_ps.shader"); }
	static std::string GetEntryPoint() { return "main"; }
};

class EdgeCubePS : public Shader<EdgeCubePS, ID3D11PixelShader>
{
public:
	struct Parameters
	{
		DirectX::XMFLOAT4 Color;
	};

	_declspec(align(16)) struct ConstantBuffer
	{
		_declspec(align(16)) DirectX::XMFLOAT4 Color;
	};

	void BindResources()
	{
		BindConstantBuffer(DX().devCon, _paramBuffer);
	}

	void SetParameters(Parameters params)
	{
		if (!_paramBuffer)
		{
			_paramBuffer = DX().CreateConstantBuffer(sizeof(ConstantBuffer));
		}

		_cachedCB.Color = params.Color;

		DX().devCon->UpdateSubresource(_paramBuffer, 0, nullptr, &_cachedCB, 0, 0);
	}

	static std::string GetFilePath() { return App().GetAbsolutePathForContent("solid_color_ps.shader"); }
	static std::string GetEntryPoint() { return "main"; }
private:
	ConstantBuffer _cachedCB;
	ID3D11Buffer* _paramBuffer;
};


class SimpleView
{
public:
	void Setup();
	void Teardown();
	void Tick(float deltaTime);
	void Draw();

	~SimpleView() {}

private:

	ID3D11Buffer* cubeVB;
	ID3D11Buffer* cubeTriangleIB;
	ID3D11Buffer* cubeEdgesIB;
	ID3D11InputLayout* vertLayout;


	CubeVS cubeVS;
	TriCubePS trianglePS;
	EdgeCubePS edgePS;

	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4X4 cubeTformMatrix;
	ID3D11RasterizerState* WireFrameRasterState;
	ID3D11RasterizerState* SolidRasterState;

	ID3D11DepthStencilState* SolidDepthStencilState;
	ID3D11DepthStencilState* WireDepthStencilState;

};
