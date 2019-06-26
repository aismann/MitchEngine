﻿#pragma once
#include <DirectXMath.h>
#include "RenderCommands.h"

#if ME_DIRECTX

// Constant buffer used to send MVP matrices to the vertex shader.
struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT3 padding;
	BOOL HasNormalMap = false;
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

#endif