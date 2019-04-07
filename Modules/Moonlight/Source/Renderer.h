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


namespace Moonlight
{
	class Renderer
	{
	public:
		struct ModelCommand
		{
			std::vector<Mesh*> Meshes;
			Shader* ModelShader;
			DirectX::XMMATRIX Transform;
		};

		void UpdateMatrix(unsigned int Id, DirectX::XMMATRIX NewTransform);
	public:
		Renderer();
		virtual ~Renderer() final;

		void Init();
		void SetWindow();
		void RegisterDeviceNotify(IDeviceNotify* deviceNotify);

		D3D12Device& GetDevice();

		void Update(float dt);
		void Render(std::function<void()> func);

		void ReleaseDeviceDependentResources();
		void CreateDeviceDependentResources();
		void InitD2DScreenTexture();

		unsigned int PushModel(const ModelCommand& model);
		bool PopModel(unsigned int id);

		void ClearModels();

		void WindowResized(const Vector2& NewSize);

	private:
		class SkyBox* Sky;
		class Plane* Grid;

		class D3D12Device* m_device;

#if ME_DIRECTX
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
		ModelViewProjectionConstantBuffer m_constantBufferData;
		float m_degreesPerSecond = 45.f;
#endif
		ID3D11SamplerState* CubesTexSamplerState;

		std::vector<ModelCommand> Models;
		std::queue<unsigned int> FreeCommandIndicies;
#if ME_ENABLE_RENDERDOC
		RenderDocManager* RenderDoc;
#endif
	};
}