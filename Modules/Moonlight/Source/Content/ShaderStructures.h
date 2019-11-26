﻿#pragma once
#include <DirectXMath.h>
#include "RenderCommands.h"

#if ME_DIRECTX
namespace Moonlight
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT2 padding;
		BOOL HasNormalMap;
		BOOL HasAlphaMap;
		BOOL HasSpecMap;
		DirectX::XMFLOAT3 DiffuseColor;
	};

	struct DepthPassBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct LightBuffer
	{
		Moonlight::LightCommand light;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};
	struct VertexPositionTexCoord
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TexCoord;
	};
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TextureCoord;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT3 BiTangent;
	};
}
#endif