#include "Renderer.h"
#include "Device/D3D12Device.h"
#include "Device/GLDevice.h"

#include "Utils/DirectXHelper.h"
#include <DirectXMath.h>
#include "Components/Camera.h"
#include <DirectXColors.h>
#include "Graphics/SkyBox.h"
#include "optick.h"
#include "Graphics/Plane.h"
#include <functional>
#include "imgui.h"
#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx11.h"
#include "Graphics/RenderTexture.h"
#include "Engine/Input.h"

using namespace DirectX;
using namespace Windows::Foundation;

namespace Moonlight
{
	void Renderer::UpdateMatrix(unsigned int Id, DirectX::XMMATRIX NewTransform)
	{
		if (Id >= Models.size())
		{
			return;
		}

		Models[Id].Transform = std::move(NewTransform);
	}

	void Renderer::UpdateMeshMatrix(unsigned int Id, DirectX::XMMATRIX NewTransform)
	{
		if (Id >= Meshes.size())
		{
			return;
		}

		Meshes[Id].Transform = std::move(NewTransform);
	}

	Renderer::Renderer()
	{
		m_device = new D3D12Device();

#if ME_ENABLE_RENDERDOC
		RenderDoc = new RenderDocManager();
#endif

		CreateDeviceDependentResources();
	}

	Renderer::~Renderer()
	{
		ReleaseDeviceDependentResources();
		delete Grid;
	}

	void Renderer::Init()
	{
		Grid = new Plane("Assets/skybox.jpg", GetDevice());

		m_defaultSampler = m_device->CreateSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP);
		m_computeSampler = m_device->CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);

		m_tonemapProgram = m_device->CreateShaderProgram(
			m_device->CompileShader(Path("Assets/Shaders/ToneMap.hlsl"), "main_vs", "vs_5_0"),
			m_device->CompileShader(Path("Assets/Shaders/ToneMap.hlsl"), "main_ps", "ps_5_0"),
			nullptr
		);

		m_depthProgram = ShaderCommand("Assets/Shaders/DepthPass.hlsl");
		ResizeBuffers();

		// Prepare the constant buffer to send it to the graphics device.
		Sunlight.dir = XMFLOAT4(0.25f, 0.5f, -1.0f, 1.0f);
		Sunlight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		Sunlight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);


		m_lightingProgram = m_device->CreateShaderProgram(
			m_device->CompileShader(Path("Assets/Shaders/LightingPass.hlsl"), "main_vs", "vs_5_0"),
			m_device->CompileShader(Path("Assets/Shaders/LightingPass.hlsl"), "main_ps", "ps_5_0"),
			nullptr
		);
	}

	void Renderer::ResizeBuffers()
	{
		delete m_framebuffer;
		delete m_resolvebuffer;
		delete SceneViewRTT;
		delete SceneResolveViewRTT;
		m_framebuffer = m_device->CreateFrameBuffer(m_device->GetOutputSize().X(), m_device->GetOutputSize().Y(), m_device->GetMSAASamples(), DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);
		SceneViewRTT = m_device->CreateFrameBuffer(m_device->GetOutputSize().X(), m_device->GetOutputSize().Y(), m_device->GetMSAASamples(), DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);
		if (m_device->GetMSAASamples() > 1)
		{
			//GameViewRTT = new RenderTexture(m_device);

			m_resolvebuffer = m_device->CreateFrameBuffer(m_device->GetOutputSize().X(), m_device->GetOutputSize().Y(), 1, DXGI_FORMAT_R16G16B16A16_FLOAT, (DXGI_FORMAT)0);
			SceneResolveViewRTT = m_device->CreateFrameBuffer(m_device->GetOutputSize().X(), m_device->GetOutputSize().Y(), 1, DXGI_FORMAT_R16G16B16A16_FLOAT, (DXGI_FORMAT)0);
		}
		else
		{
			SceneResolveViewRTT = SceneViewRTT;
			m_resolvebuffer = m_framebuffer;
		}
	}

	void Renderer::SetWindow()
	{

	}

	void Renderer::RegisterDeviceNotify(IDeviceNotify* deviceNotify)
	{

	}

	D3D12Device& Renderer::GetDevice()
	{
		return *m_device;
	}

	void Renderer::Update(float dt)
	{
	}

	void Renderer::Render(std::function<void()> func, const CameraData& mainCamera, const CameraData& editorCamera)
	{
		OPTICK_EVENT("Renderer::Render", Optick::Category::Rendering);

		auto context = m_device->GetD3DDeviceContext();

		//float darkGrey = (0.0f / 255.0f);
		DirectX::XMVECTORF32 color = Colors::Black;//{ {darkGrey, darkGrey, darkGrey, 1.0f} };

		// Clear the back buffer and depth stencil view.
		context->ClearRenderTargetView(m_device->GetBackBufferRenderTargetView(), color);
		context->ClearRenderTargetView(m_framebuffer->RenderTargetView.Get(), color);
		context->ClearRenderTargetView(m_framebuffer->ColorRenderTargetView.Get(), color);
		context->ClearRenderTargetView(m_framebuffer->NormalRenderTargetView.Get(), color);
		context->ClearRenderTargetView(m_framebuffer->SpecularRenderTargetView.Get(), color);

		context->ClearRenderTargetView(SceneViewRTT->RenderTargetView.Get(), color);
		context->ClearRenderTargetView(SceneViewRTT->ColorRenderTargetView.Get(), color);
		context->ClearRenderTargetView(SceneViewRTT->NormalRenderTargetView.Get(), color);
		context->ClearRenderTargetView(SceneViewRTT->SpecularRenderTargetView.Get(), color);

		context->ClearDepthStencilView(m_framebuffer->DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		CD3D11_VIEWPORT gameViewport = CD3D11_VIEWPORT(
			0.0f,
			0.0f,
			mainCamera.OutputSize.X(),
			mainCamera.OutputSize.Y()
		);
		context->RSSetViewports(1, &gameViewport);

		m_device->GetD3DDeviceContext()->OMSetBlendState(0, 0, 0xffffffff);

		// Reset render targets to the screen.

		DrawScene(context, m_constantBufferData, mainCamera, m_framebuffer, m_resolvebuffer);


		context->ClearDepthStencilView(SceneViewRTT->DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

#if ME_EDITOR

		//ID3D11RenderTargetView* const targets2[1] = { SceneViewRTT->renderTargetViewMap };
		DrawScene(context, m_constantBufferSceneData, editorCamera, SceneViewRTT, SceneResolveViewRTT);


		//context->ClearRenderTargetView(SceneViewRTT->RenderTargetView.Get(), color);
		//ID3D11RenderTargetView** arr[1] = {  };
		//context->OMSetRenderTargets(1, m_device->m_d3dRenderTargetView.GetAddressOf(), nullptr);

		//GetDevice().GetD3DDeviceContext()->OMSetRenderTargets(1, m_device->m_d3dRenderTargetView.GetAddressOf(), nullptr);
		// Prepare the constant buffer to send it to the graphics device.


		//// Attach our vertex shader.
		//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//UINT stride = sizeof(PositionVertex);
		//UINT offset = 0;
		//context->IASetVertexBuffers(
		//	0,
		//	1,
		//	m_vertexBuffer.GetAddressOf(),
		//	&stride,
		//	&offset
		//);

		//context->IASetIndexBuffer(
		//	m_indexBuffer.Get(),
		//	DXGI_FORMAT_R32_UINT, // Each index is one 16-bit unsigned integer (short).
		//	0
		//);
		//context->VSSetShader(
		//	m_lightingProgram.VertexShader.Get(),
		//	nullptr,
		//	0
		//);
		//// Attach our pixel shader.
		//context->PSSetShader(
		//	m_lightingProgram.PixelShader.Get(),
		//	nullptr,
		//	0
		//);

		//{
		//	OPTICK_EVENT("Mesh::Draw::Setup");
		//}
		//{
		//	{
		//		OPTICK_EVENT("Mesh::Draw::Texture::ShaderResources");
		//		context->PSSetShaderResources(0, 1, &SceneViewRTT->ShaderResourceView);
		//		//if (mat->GetTexture(TextureType::Normal))
		//		{
		//			//context->PSSetShaderResources(1, 1, &SceneViewRTT->NormalShaderResourceView);
		//		}
		//		context->PSSetSamplers(0, 1, m_defaultSampler.GetAddressOf());
		//	}
		//}

		//{
		//	OPTICK_EVENT("Mesh::Draw::DrawCall");
		//	// Draw the objects.
		//	context->DrawIndexed(
		//		6,
		//		0,
		//		0
		//	);
		//}

		//context->OMSetRenderTargets(1, m_device->m_d3dRenderTargetView.GetAddressOf(), nullptr);
		//context->IASetInputLayout(nullptr);
		//context->VSSetShader(m_tonemapProgram.VertexShader.Get(), nullptr, 0);
		//context->PSSetShader(m_tonemapProgram.PixelShader.Get(), nullptr, 0);
		//context->PSSetShaderResources(0, 1, m_resolvebuffer->ShaderResourceView.GetAddressOf());
		//context->PSSetSamplers(0, 1, m_computeSampler.GetAddressOf());
		//context->Draw(3, 0);

		// Scene grid
		//{
		//	XMStoreFloat4x4(&m_constantBufferSceneData.model, XMMatrixTranspose(XMMatrixIdentity()));
		//	// Prepare the constant buffer to send it to the graphics device.
		//	context->UpdateSubresource1(
		//		m_constantBuffer.Get(),
		//		0,
		//		NULL,
		//		&m_constantBufferSceneData,
		//		0,
		//		0,
		//		0
		//	);

		//	// Send the constant buffer to the graphics device.
		//	context->VSSetConstantBuffers1(
		//		0,
		//		1,
		//		m_constantBuffer.GetAddressOf(),
		//		nullptr,
		//		nullptr
		//	);

		//	Grid->Draw(GetDevice());
		//}

#endif

		GetDevice().GetD3DDeviceContext()->OMSetRenderTargets(1, m_device->m_d3dRenderTargetView.GetAddressOf(), nullptr);

		func();

		// The first argument instructs DXGI to block until VSync, putting the application
		// to sleep until the next VSync. This ensures we don't waste any cycles rendering
		// frames that will never be displayed to the screen.
		DXGI_PRESENT_PARAMETERS parameters = { 0 };
		HRESULT hr = m_device->GetSwapChain()->Present1(0, 0, &parameters);

		// Discard the contents of the render target.
		// This is a valid operation only when the existing contents will be entirely
		// overwritten. If dirty or scroll rects are used, this call should be removed.
		m_device->GetD3DDeviceContext()->DiscardView1(m_device->GetBackBufferRenderTargetView(), nullptr, 0);
		m_device->GetD3DDeviceContext()->DiscardView1(m_resolvebuffer->ShaderResourceView.Get(), nullptr, 0);
		m_device->GetD3DDeviceContext()->DiscardView1(m_resolvebuffer->ColorShaderResourceView.Get(), nullptr, 0);
		m_device->GetD3DDeviceContext()->DiscardView1(m_resolvebuffer->NormalShaderResourceView.Get(), nullptr, 0);
		m_device->GetD3DDeviceContext()->DiscardView1(m_resolvebuffer->SpecularShaderResourceView.Get(), nullptr, 0);
		m_device->GetD3DDeviceContext()->DiscardView1(SceneResolveViewRTT->ShaderResourceView.Get(), nullptr, 0);
		m_device->GetD3DDeviceContext()->DiscardView1(SceneResolveViewRTT->ColorShaderResourceView.Get(), nullptr, 0);
		m_device->GetD3DDeviceContext()->DiscardView1(SceneResolveViewRTT->NormalShaderResourceView.Get(), nullptr, 0);
		m_device->GetD3DDeviceContext()->DiscardView1(SceneResolveViewRTT->SpecularShaderResourceView.Get(), nullptr, 0);

		// Discard the contents of the depth stencil.
		//m_device->GetD3DDeviceContext()->DiscardView1(m_framebuffer->DepthStencilView.Get(), nullptr, 0);

		// If the device was removed either by a disconnection or a driver upgrade, we 
		// must recreate all device resources.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			m_device->HandleDeviceLost();
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}


	void Renderer::DrawScene(ID3D11DeviceContext3* context, ModelViewProjectionConstantBuffer& constantBufferSceneData, const CameraData& camera, FrameBuffer* ViewRTT, FrameBuffer* ResolveViewRTT)
	{
		Vector2 outputSize = camera.OutputSize;
		if (outputSize.IsZero())
		{
			return;
		}
		float aspectRatio = static_cast<float>(outputSize.X()) / static_cast<float>(outputSize.Y());
		float fovAngleY = camera.FOV * XM_PI / 180.0f;


		// This is a simple example of change that can be made when the app is in
		// portrait or snapped view.
		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		// Note that the OrientationTransform3D matrix is post-multiplied here
		// in order to correctly orient the scene to match the display orientation.
		// This post-multiplication step is required for any draw calls that are
		// made to the swap chain render target. For draw calls to other targets,
		// this transform should not be applied.

		// This sample makes use of a right-handed coordinate system using row-major matrices.
		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
			fovAngleY,
			aspectRatio,
			.1f,
			1000.0f
		);

		XMStoreFloat4x4(
			&constantBufferSceneData.projection,
			XMMatrixTranspose(perspectiveMatrix)
		);

		const XMVECTORF32 eye = { camera.Position.X(), camera.Position.Y(), camera.Position.Z(), 0 };
		const XMVECTORF32 at = { camera.Position.X() + camera.Front.X(), camera.Position.Y() + camera.Front.Y(), camera.Position.Z() + camera.Front.Z(), 0.0f };
		const XMVECTORF32 up = { camera.Up.X(), camera.Up.Y(), camera.Up.Z(), 0 };

		ID3D11RenderTargetView** targets[3] = { ViewRTT->ColorRenderTargetView.GetAddressOf(), ViewRTT->NormalRenderTargetView.GetAddressOf(), ViewRTT->SpecularRenderTargetView.GetAddressOf() };
		context->OMSetRenderTargets(3, targets[0], ViewRTT->DepthStencilView.Get());

		CD3D11_VIEWPORT screenViewport = CD3D11_VIEWPORT(
			0.0f,
			0.0f,
			camera.OutputSize.X(),
			camera.OutputSize.Y()
		);

		context->RSSetViewports(1, &screenViewport);

		//XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixIdentity()));
		XMStoreFloat4x4(&constantBufferSceneData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

		if (camera.Skybox)
		{
			XMStoreFloat4x4(&constantBufferSceneData.model, XMMatrixTranspose(XMMatrixTranslation(eye[0], eye[1], eye[2])));
			// Prepare the constant buffer to send it to the graphics device.
			context->UpdateSubresource1(
				m_constantBuffer.Get(),
				0,
				NULL,
				&constantBufferSceneData,
				0,
				0,
				0
			);

			// Send the constant buffer to the graphics device.
			context->VSSetConstantBuffers1(
				0,
				1,
				m_constantBuffer.GetAddressOf(),
				nullptr,
				nullptr
			);
			camera.Skybox->Draw();
		}

		m_device->ResetCullingMode();

		if (Lights.size() > 0)
		{
			//constantBufferSceneData.light = Lights[0];
		}

		{
			std::vector<MeshCommand> transparentMeshes;
			for (const MeshCommand& mesh : Meshes)
			{
				if (mesh.MeshMaterial && mesh.MeshMaterial->IsTransparent())
				{
					transparentMeshes.push_back(mesh);
					continue;
				}

				if (mesh.MeshShader && mesh.SingleMesh && mesh.MeshMaterial)
				{
					OPTICK_EVENT("Render::ModelCommand", Optick::Category::Rendering);
					XMStoreFloat4x4(&constantBufferSceneData.model, XMMatrixTranspose(mesh.Transform));
					if (mesh.MeshMaterial->GetTexture(TextureType::Normal))
					{
						constantBufferSceneData.HasNormalMap = TRUE;
					}
					else
					{
						constantBufferSceneData.HasNormalMap = FALSE;
					}
					if (mesh.MeshMaterial->GetTexture(TextureType::Specular))
					{
						constantBufferSceneData.HasSpecMap = TRUE;
					}
					else
					{
						constantBufferSceneData.HasSpecMap = TRUE;
					}
					constantBufferSceneData.HasAlphaMap = FALSE;

					// Prepare the constant buffer to send it to the graphics device.
					context->UpdateSubresource1(
						m_constantBuffer.Get(),
						0,
						NULL,
						&constantBufferSceneData,
						0,
						0,
						0
					);

					// Send the constant buffer to the graphics device.
					context->VSSetConstantBuffers1(
						0,
						1,
						m_constantBuffer.GetAddressOf(),
						nullptr,
						nullptr
					);
					context->PSSetConstantBuffers1(
						0,
						1,
						m_constantBuffer.GetAddressOf(),
						nullptr,
						nullptr
					);
					OPTICK_EVENT("Render::SingleMesh");

					mesh.MeshShader->Use();

					mesh.SingleMesh->Draw(mesh.MeshMaterial);
				}
			}

			m_device->GetD3DDeviceContext()->OMSetBlendState(m_device->TransparentBlendState, Colors::Black, 0xffffffff);

			for (const MeshCommand& mesh : transparentMeshes)
			{
				if (mesh.MeshShader && mesh.SingleMesh && mesh.MeshMaterial)
				{
					OPTICK_EVENT("Render::ModelCommand", Optick::Category::Rendering);
					XMStoreFloat4x4(&constantBufferSceneData.model, XMMatrixTranspose(mesh.Transform));
					if (mesh.MeshMaterial->GetTexture(TextureType::Normal))
					{
						constantBufferSceneData.HasNormalMap = TRUE;
					}
					else
					{
						constantBufferSceneData.HasNormalMap = FALSE;
					}
					if (mesh.MeshMaterial->GetTexture(TextureType::Specular))
					{
						constantBufferSceneData.HasSpecMap = TRUE;
					}
					else
					{
						constantBufferSceneData.HasSpecMap = TRUE;
					}
					constantBufferSceneData.HasAlphaMap = TRUE;

					// Prepare the constant buffer to send it to the graphics device.
					context->UpdateSubresource1(
						m_constantBuffer.Get(),
						0,
						NULL,
						&constantBufferSceneData,
						0,
						0,
						0
					);

					// Send the constant buffer to the graphics device.
					context->VSSetConstantBuffers1(
						0,
						1,
						m_constantBuffer.GetAddressOf(),
						nullptr,
						nullptr
					);
					context->PSSetConstantBuffers1(
						0,
						1,
						m_constantBuffer.GetAddressOf(),
						nullptr,
						nullptr
					);
					OPTICK_EVENT("Render::SingleAlphaMesh");

					mesh.MeshShader->Use();

					mesh.SingleMesh->Draw(mesh.MeshMaterial);
				}
			}
		}
		m_device->GetD3DDeviceContext()->OMSetBlendState(0, 0, 0xffffffff);

		CD3D11_VIEWPORT finalGameRenderViewport = CD3D11_VIEWPORT(
			0.0f,
			0.0f,
			ViewRTT->Width,
			ViewRTT->Height
		);
		context->RSSetViewports(1, &finalGameRenderViewport);
		//m_perFrameBufferData.light = light;
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->UpdateSubresource1(m_perFrameBuffer.Get(), 0, NULL, &Sunlight, 0, 0, 0);
		context->PSSetConstantBuffers1(0, 1, m_perFrameBuffer.GetAddressOf(), nullptr, nullptr);
		// post stuff
		context->OMSetRenderTargets(1, ViewRTT->RenderTargetView.GetAddressOf(), nullptr);
		context->IASetInputLayout(nullptr);
		context->VSSetShader(m_lightingProgram.VertexShader.Get(), nullptr, 0);
		context->PSSetShader(m_lightingProgram.PixelShader.Get(), nullptr, 0);
		context->PSSetShaderResources(0, 1, ResolveViewRTT->ColorShaderResourceView.GetAddressOf());
		context->PSSetShaderResources(1, 1, ResolveViewRTT->NormalShaderResourceView.GetAddressOf());
		context->PSSetShaderResources(2, 1, ResolveViewRTT->SpecularShaderResourceView.GetAddressOf());
		context->PSSetShaderResources(3, 1, ViewRTT->DepthShaderResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, m_computeSampler.GetAddressOf());
		context->Draw(3, 0);

		if (ViewRTT->FinalTexture != ResolveViewRTT->FinalTexture)
		{
			context->ResolveSubresource(ResolveViewRTT->FinalTexture.Get(), 0, ViewRTT->FinalTexture.Get(), 0, DXGI_FORMAT_R16G16B16A16_FLOAT);
		}
		if (ViewRTT->ColorTexture != ResolveViewRTT->ColorTexture)
		{
			context->ResolveSubresource(ResolveViewRTT->ColorTexture.Get(), 0, ViewRTT->ColorTexture.Get(), 0, DXGI_FORMAT_R16G16B16A16_FLOAT);
		}
		if (ViewRTT->NormalTexture != ResolveViewRTT->NormalTexture)
		{
			context->ResolveSubresource(ResolveViewRTT->NormalTexture.Get(), 0, ViewRTT->NormalTexture.Get(), 0, DXGI_FORMAT_R16G16B16A16_FLOAT);
		}
		if (ViewRTT->SpecularTexture != ResolveViewRTT->SpecularTexture)
		{
			context->ResolveSubresource(ResolveViewRTT->SpecularTexture.Get(), 0, ViewRTT->SpecularTexture.Get(), 0, DXGI_FORMAT_R16G16B16A16_FLOAT);
		}

		if (Input::GetInstance().IsKeyDown(KeyCode::A))
		{
			context->UpdateSubresource1(m_perFrameBuffer.Get(), 0, NULL, &Sunlight, 0, 0, 0);
			context->PSSetConstantBuffers1(0, 1, m_perFrameBuffer.GetAddressOf(), nullptr, nullptr);
			// post stuff
			context->OMSetRenderTargets(1, ViewRTT->RenderTargetView.GetAddressOf(), nullptr);
			context->IASetInputLayout(nullptr);
			context->VSSetShader(m_tonemapProgram.VertexShader.Get(), nullptr, 0);
			context->PSSetShader(m_tonemapProgram.PixelShader.Get(), nullptr, 0);
			context->PSSetShaderResources(0, 1, ResolveViewRTT->ShaderResourceView.GetAddressOf());
			context->PSSetSamplers(0, 1, m_computeSampler.GetAddressOf());
			context->Draw(3, 0);
			if (ViewRTT->FinalTexture != ResolveViewRTT->FinalTexture)
			{
				context->ResolveSubresource(ResolveViewRTT->FinalTexture.Get(), 0, ViewRTT->FinalTexture.Get(), 0, DXGI_FORMAT_R16G16B16A16_FLOAT);
			}
		}

	}

	void Renderer::ReleaseDeviceDependentResources()
	{
		m_constantBuffer.Reset();
		m_perFrameBuffer.Reset();
	}

	void Renderer::CreateDeviceDependentResources()
	{
		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			static_cast<D3D12Device*>(m_device)->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);

		CD3D11_BUFFER_DESC perFrameBufferDesc(sizeof(LightCommand), D3D11_BIND_CONSTANT_BUFFER);
		/*CD3D11_BUFFER_DESC perFrameBufferDesc;
		ZeroMemory(&perFrameBufferDesc, sizeof(D3D11_BUFFER_DESC));

		perFrameBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		perFrameBufferDesc.ByteWidth = sizeof(LightCommand);
		perFrameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perFrameBufferDesc.CPUAccessFlags = 0;
		perFrameBufferDesc.MiscFlags = 0;*/
		DX::ThrowIfFailed(
			static_cast<D3D12Device*>(m_device)->GetD3DDevice()->CreateBuffer(&perFrameBufferDesc, nullptr, &m_perFrameBuffer)
		);
		//hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerFrameBuffer);
	}

	unsigned int Renderer::PushModel(const ModelCommand& NewModel)
	{
		if (!FreeCommandIndicies.empty())
		{
			unsigned int openIndex = FreeCommandIndicies.front();
			FreeCommandIndicies.pop();
			Models[openIndex] = std::move(NewModel);
			return openIndex;
		}

		Models.push_back(std::move(NewModel));
		return static_cast<unsigned int>(Models.size() - 1);
	}

	bool Renderer::PopModel(unsigned int id)
	{
		if (id >= Models.size())
		{
			return false;
		}

		Models[id].Meshes.clear();
		return true;
	}

	void Renderer::ClearModels()
	{
		Models.clear();
		while (!FreeCommandIndicies.empty())
		{
			FreeCommandIndicies.pop();
		}
	}

	unsigned int Renderer::PushLight(const LightCommand& NewLight)
	{
		if (!FreeLightCommandIndicies.empty())
		{
			unsigned int openIndex = FreeLightCommandIndicies.front();
			FreeLightCommandIndicies.pop();
			Lights[openIndex] = std::move(NewLight);
			return openIndex;
		}

		Lights.push_back(std::move(NewLight));
		return static_cast<unsigned int>(Lights.size() - 1);
	}

	bool Renderer::PopLight(unsigned int id)
	{
		if (id >= Lights.size())
		{
			return false;
		}

		// Reset struct

		return true;
	}

	void Renderer::ClearLights()
	{
		Lights.clear();
		while (!FreeLightCommandIndicies.empty())
		{
			FreeLightCommandIndicies.pop();
		}
	}

	void Renderer::WindowResized(const Vector2& NewSize)
	{
		if (!m_device)
		{
			return;
		}

		m_device->WindowSizeChanged(NewSize);
		ResizeBuffers();
	}

	unsigned int Renderer::PushMesh(Moonlight::MeshCommand command)
	{
		if (!FreeMeshCommandIndicies.empty())
		{
			unsigned int openIndex = FreeMeshCommandIndicies.front();
			FreeMeshCommandIndicies.pop();
			Meshes[openIndex] = std::move(command);
			return openIndex;
		}

		Meshes.push_back(std::move(command));
		return static_cast<unsigned int>(Meshes.size() - 1);
	}

	void Renderer::ClearMeshes()
	{
		Meshes.clear();
		while (!FreeMeshCommandIndicies.empty())
		{
			FreeMeshCommandIndicies.pop();
		}
	}

	void Renderer::PopMesh(unsigned int Id)
	{
		if (Id > Meshes.size())
		{
			return;
		}

		FreeMeshCommandIndicies.push(Id);
		Meshes[Id] = MeshCommand();
	}
}
