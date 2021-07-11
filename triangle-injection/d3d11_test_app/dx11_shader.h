#pragma once
#include "dX11_render_backend.h"
#include "common_header.h"
#include <string>

enum class EShaderType : uint8_t
{
	VertexShader,
	PixelShader,
	ComputeShader,
	GeometryShader,
	Unsupported
};

template<typename ShaderConcreteType, typename D3DShaderType>
class Shader
{
public:

	Shader()
	{
		static_assert(std::is_base_of<ID3D11DeviceChild, D3DShaderType>::value, "Invalid Shader Type Specified");
		Compile();
	}

	~Shader()
	{
		_compiledShader->Release();
		_blob->Release();
	}

	void Compile()
	{
		HRESULT err;
		ID3D10Blob* compileErrors;

		if constexpr (std::is_same<ID3D11VertexShader, D3DShaderType>::value) 
		{
			bool compileErr = CompileBlob(GetFilePathInternal(), GetEntryPointInternal(), "vs_5_0", &_blob, &compileErrors);
			check(compileErr);
			err = DX().device->CreateVertexShader(_blob->GetBufferPointer(), _blob->GetBufferSize(), NULL, &_compiledShader);
		}
		else if constexpr (std::is_same<ID3D11PixelShader, D3DShaderType>::value)
		{
			bool compileErr = CompileBlob(GetFilePathInternal(), GetEntryPointInternal(), "ps_5_0", &_blob, &compileErrors);
			check(compileErr);
			err = DX().device->CreatePixelShader(_blob->GetBufferPointer(), _blob->GetBufferSize(), NULL, &_compiledShader);
		}
		else if constexpr (std::is_same<ID3D11ComputeShader, D3DShaderType>::value)
		{
			bool compileErr = CompileBlob(GetFilePathInternal(), GetEntryPointInternal(), "cs_5_0", &_blob, &compileErrors);
			check(compileErr);
			err = DX().device->CreateComputeShader(_blob->GetBufferPointer(), _blob->GetBufferSize(), NULL, &_compiledShader);
		}
		else if constexpr (std::is_same<ID3D11GeometryShader, D3DShaderType>::value)
		{
			bool compileErr = CompileBlob(GetFilePathInternal(), GetEntryPointInternal(), "gs_5_0", &_blob, &compileErrors);
			check(compileErr);
			err = DX().device->CreateGeometryShader(_blob->GetBufferPointer(), _blob->GetBufferSize(), NULL, &_compiledShader);
		}
		check(err == S_OK);
	}

	void BindResources()
	{
		static_cast<ShaderConcreteType*>(this)->BindResources();
	}

	D3DShaderType* Get()
	{
		check(_compiledShader);
		return _compiledShader;
	}

	constexpr EShaderType GetShaderType()
	{
		if (std::is_same<ID3D11VertexShader, D3DShaderType>::value)
		{
			return EShaderType::VertexShader;
		}
		else if (std::is_same<ID3D11PixelShader, D3DShaderType>::value)
		{
			return EShaderType::PixelShader;
		}
		else if (std::is_same<ID3D11ComputeShader, D3DShaderType>::value)
		{
			return EShaderType::ComputeShader;
		}
		else if (std::is_same<ID3D11GeometryShader, D3DShaderType>::value)
		{
			return EShaderType::GeometryShader;
		}


		return EShaderType::Unsupported;
	}

	void Use(ID3D11DeviceContext* devCon)
	{
		BindResources();

		if constexpr (std::is_same<ID3D11VertexShader, D3DShaderType>::value)
		{
			devCon->VSSetShader(Get(), 0, 0);
		}
		else if constexpr (std::is_same<ID3D11PixelShader, D3DShaderType>::value)
		{
			devCon->PSSetShader(Get(), 0, 0);
		}
		else if constexpr (std::is_same<ID3D11ComputeShader, D3DShaderType>::value)
		{
			devCon->CSSetShader(Get(), 0, 0);
		}
		else if constexpr (std::is_same<ID3D11GeometryShader, D3DShaderType>::value)
		{
			devCon->GSSetShader(Get(), 0, 0);
		}

	}

	ID3D10Blob* GetBytecode() const { return _blob; }

	void BindConstantBuffer(ID3D11DeviceContext* devCon, ID3D11Buffer* buffers, uint32_t startSlot = 0, uint32_t numBuffers = 1)
	{
		if constexpr (std::is_same<ID3D11VertexShader, D3DShaderType>::value)
		{
			devCon->VSSetConstantBuffers(startSlot, numBuffers, &buffers);
		}
		else if constexpr (std::is_same<ID3D11PixelShader, D3DShaderType>::value)
		{
			devCon->PSSetConstantBuffers(startSlot, numBuffers, &buffers);
		}
		else if constexpr (std::is_same<ID3D11ComputeShader, D3DShaderType>::value)
		{
			devCon->CSSetConstantBuffers(startSlot, numBuffers, &buffers);
		}
		else if constexpr (std::is_same<ID3D11GeometryShader, D3DShaderType>::value)
		{
			devCon->GSSetConstantBuffers(startSlot, numBuffers, &buffers);
		}
	}

	void BindSRV(ID3D11DeviceContext* devCon, ID3D11ShaderResourceView* SRVs, uint32_t startSlot = 0, uint32_t numSRVs = 1)
	{

		if constexpr (std::is_same<ID3D11ComputeShader, D3DShaderType>::value)
		{
			devCon->CSSetShaderResources(startSlot, numSRVs, &SRVs);
		}
		else if constexpr (std::is_same<ID3D11PixelShader, D3DShaderType>::value)
		{
			devCon->PSSetShaderResources(startSlot, numSRVs, &SRVs);
		}
		else
		{
			static_assert(0, "SRV binding is only supported for compute and pixel shaders");
		}
	}


	void BindUAV(ID3D11DeviceContext* devCon, ID3D11UnorderedAccessView* UAVs, uint32_t startSlot = 0, uint32_t numUAVs = 1)
	{
		if constexpr (std::is_same<ID3D11ComputeShader, D3DShaderType>::value)
		{
			devCon->CSSetUnorderedAccessViews(startSlot, numUAVs, &UAVs, nullptr);
		}
		else if constexpr (std::is_same<ID3D11PixelShader, D3DShaderType>::value)
		{
			devCon->CSSetUnorderedAccessViews(startSlot, numUAVs, &UAVs, nullptr);
		}
		else
		{
			static_assert(0, "BinUAV is only supported for compute and pixel shaders");
		}
	}

	//don't assert directly in this function if compilation fails, since we don't want to crash if a shader reload doesn't compile
	bool CompileBlob(std::string filepath, std::string entrypoint, std::string shadermodel, ID3D10Blob** outBlob, ID3D10Blob** outErrors)
	{
		std::wstring wPath(filepath.size() + 1, L' ');
		size_t outSize;

		mbstowcs_s(&outSize, &wPath[0], filepath.size() + 1, filepath.c_str(), filepath.size());

		HRESULT err = D3DCompileFromFile(wPath.c_str(), 0, 0, entrypoint.c_str(), shadermodel.c_str(), 0, 0, outBlob, outErrors);
		if (outErrors != nullptr && *outErrors)
		{
			ID3D10Blob* outErrorsDeref = *outErrors;
			OutputDebugStringA((char*)outErrorsDeref->GetBufferPointer());
		}

		return err == S_OK;
	}

private:

	std::string GetFilePathInternal()
	{
		return ShaderConcreteType::GetFilePath();
	}

	std::string GetEntryPointInternal()
	{
		return ShaderConcreteType::GetEntryPoint();
	}

protected:
	ID3D10Blob* _blob = nullptr;
	D3DShaderType* _compiledShader = nullptr;
};

