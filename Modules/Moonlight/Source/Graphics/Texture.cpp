#include <iostream>
#include <assert.h>
#include "Texture.h"
#include "Logger.h"
#include <WICTextureLoader.h>
#include "Game.h"
#include "Device/D3D12Device.h"
#include "Engine/Engine.h"
#include "Utils/StringUtils.h"

using namespace DirectX;
using namespace Microsoft::WRL;


namespace Moonlight
{
	Texture::Texture(const Path& InFilePath, WrapMode mode)
		: Resource(InFilePath)
	{
		std::wstring filePath = StringUtils::ToWString(InFilePath.FullPath);
		auto& device = static_cast<Moonlight::D3D12Device&>(GetEngine().GetRenderer().GetDevice());
		ID3D11DeviceContext* context;
		device.GetD3DDevice()->GetImmediateContext(&context);

		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		UINT miscFlags = 0;
		//if (levels == 0)
		{
			bindFlags |= D3D11_BIND_RENDER_TARGET;
			miscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
		DX::ThrowIfFailed(CreateWICTextureFromFileEx(device.GetD3DDevice(), context, filePath.c_str(), 2048, D3D11_USAGE_DEFAULT, bindFlags, NULL, miscFlags, NULL, &resource, &ShaderResourceView));

		D3D11_TEXTURE_ADDRESS_MODE dxMode = D3D11_TEXTURE_ADDRESS_WRAP;
		switch (mode)
		{
		case Moonlight::Clamp:
		case Moonlight::Decal:
			dxMode = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case Moonlight::Wrap:
			dxMode = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case Moonlight::Mirror:
			dxMode = D3D11_TEXTURE_ADDRESS_MIRROR;
			break;
		default:
			break;
		}
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = dxMode;
		sampDesc.AddressV = dxMode;
		sampDesc.AddressW = dxMode;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		device.GetD3DDevice()->CreateSamplerState(&sampDesc, &SamplerState);
	}

	Texture::~Texture()
	{
		// TODO: Unload textures
	}

	std::string Texture::ToString(TextureType type)
	{
		switch (type)
		{
		case TextureType::Diffuse:
			return "Diffuse";
		case TextureType::Normal:
			return "Normal";
		case TextureType::Specular:
			return "Specular";
		case TextureType::Height:
			return "Height";
		case TextureType::Opacity:
			return "Opacity";
		case TextureType::Count:
		default:
			Logger::GetInstance().Log(Logger::LogType::Error, "Couldn't find texture type: " + std::to_string(type));
			return "";
		}
	}

}