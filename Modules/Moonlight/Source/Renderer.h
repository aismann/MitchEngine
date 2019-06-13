#pragma once
#include <memory>

#include "Device/IDevice.h"
#include "Singleton.h"
#include "Resource/ResourceCache.h"
#include "Graphics/ModelResource.h"
#include "Utils/DirectXHelper.h"

#if ME_ENABLE_RENDERDOC
#include "Debug/RenderDocManager.h"
#endif
#include <d3d11.h>
#include <DirectXMath.h>
#include <queue>
#include "Math/Vector2.h"
#include <functional>
#include "RenderCommands.h"

namespace Moonlight
{
	class Renderer
	{
	public:

		void UpdateMatrix(unsigned int Id, DirectX::XMMATRIX NewTransform);
		void UpdateMeshMatrix(unsigned int Id, DirectX::XMMATRIX NewTransform);
	public:
		Renderer();
		virtual ~Renderer() final;

		void Init();

		void SetWindow();
		void RegisterDeviceNotify(IDeviceNotify* deviceNotify);

		D3D12Device& GetDevice();

		void Update(float dt);
		void Render(std::function<void()> func, const CameraData& mainCamera, const CameraData& editorCamera);

		void DrawScene(ID3D11DeviceContext3* context, ModelViewProjectionConstantBuffer& constantBufferSceneData, const CameraData& data);

		void ReleaseDeviceDependentResources();
		void CreateDeviceDependentResources();
		void InitD2DScreenTexture();

		unsigned int PushModel(const ModelCommand& model);
		bool PopModel(unsigned int id);

		void ClearModels();

		unsigned int PushLight(const LightCommand& NewLight);
		bool PopLight(unsigned int id);

		void ClearLights();

		void WindowResized(const Vector2& NewSize);

		//class RenderTexture* GameViewRTT = nullptr;
		FrameBuffer* SceneViewRTT = nullptr;
		FrameBuffer* SceneResolveViewRTT = nullptr;
		FrameBuffer* m_framebuffer = nullptr;
		FrameBuffer* m_resolvebuffer = nullptr;
		LightCommand Sunlight;

		ShaderProgram m_tonemapProgram;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_computeSampler;
	private:
		class SkyBox* Sky;
		class Plane* Grid;

		class D3D12Device* m_device = nullptr;

		void ResizeBuffers();
#if ME_DIRECTX
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_perFrameBuffer;
		ModelViewProjectionConstantBuffer m_constantBufferData;
		ModelViewProjectionConstantBuffer m_constantBufferSceneData;

		LightBuffer m_perFrameBufferData;
#endif

		std::vector<ModelCommand> Models;
		std::queue<unsigned int> FreeCommandIndicies;

		std::vector<LightCommand> Lights;
		std::queue<unsigned int> FreeLightCommandIndicies;

#if ME_ENABLE_RENDERDOC
		RenderDocManager* RenderDoc;
#endif
	public:
		unsigned int PushMesh(Moonlight::MeshCommand command);
		void PopMesh(unsigned int Id);
		void ClearMeshes();
		std::vector<MeshCommand> Meshes;
		std::queue<unsigned int> FreeMeshCommandIndicies;
	};
}