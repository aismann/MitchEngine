#include "ShaderCommand.h"

#include "Utils/DirectXHelper.h"
#include "LegacyRenderer.h"
#include "Game.h"

#include <VertexTypes.h>
#include "Texture.h"
#include "optick.h"
#include "Graphics/ShaderFile.h"
#include "Resource/ResourceCache.h"
#include "Engine/Engine.h"
#include "Pointers.h"

namespace Moonlight
{
	//hlsl files - ShaderFile
	//shader asset - shader
	//shader backend - shaderdata
	//
	//shader has a shader file and shader data
	//
	//shader has shader type
	//
	//editor loads mesh with default shader
	//	shader stuff exported on mesh component
	//
	//mesh loads, created default shader
	//, editor loads on top points to new shader

	ShaderCommand::ShaderCommand(const std::string& InShaderFile)
	{
		OPTICK_EVENT("ShaderCommand(string)");
		// REWRITE
		//auto& dxDevice = static_cast<LegacyDX11Device&>(GetEngine().GetRenderer().GetDevice());

		//if (!dxDevice.FindShader(InShaderFile, Program))
		//{
		//	auto vs = dxDevice.CompileShader(Path(InShaderFile), "main_vs", "vs_4_0_level_9_3");
		//	auto ps = dxDevice.CompileShader(Path(InShaderFile), "main_ps", "ps_4_0_level_9_3");

		//	// Wrap this and subscribe to 
		//	static const std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc =
		//	{
		//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//		{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	};

		//	Program = dxDevice.CreateShaderProgram(InShaderFile, vs, ps, &vertexDesc);
		//}

		isLoaded = true;
	}

	ShaderCommand::~ShaderCommand()
	{
	}

	void ShaderCommand::Use()
	{
		OPTICK_EVENT("Shader::Use", Optick::Category::Rendering)

		if (!IsLoaded())
		{
			return;
		}
		// REWRITE
		//auto& dxDevice = static_cast<LegacyDX11Device&>(GetEngine().GetRenderer().GetDevice());
		//dxDevice.GetD3DDeviceContext()->IASetInputLayout(Program.InputLayout.Get());

		//// Attach our vertex shader.
		//dxDevice.GetD3DDeviceContext()->VSSetShader(
		//	Program.VertexShader.Get(),
		//	nullptr,
		//	0
		//);
		//// Attach our pixel shader.
		//dxDevice.GetD3DDeviceContext()->PSSetShader(
		//	Program.PixelShader.Get(),
		//	nullptr,
		//	0
		//);
	}

	const ShaderProgram& ShaderCommand::GetProgram() const
	{
		return Program;
	}

	void ShaderCommand::SetMat4(const std::string &name, const Matrix4& mat) const
	{
	}

	void ShaderCommand::SetInt(const std::string &name, int value) const
	{
	}

	void ShaderCommand::SetVec3(const std::string &name, const Vector3& value) const
	{
	}

	void ShaderCommand::SetVec3(const std::string &name, float x, float y, float z) const
	{
	}

	void ShaderCommand::SetFloat(const std::string &name, float value) const
	{
	}
}