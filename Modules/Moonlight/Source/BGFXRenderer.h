#pragma once
#include <bgfx/bgfx.h>
#include <Math/Vector2.h>
#include "Camera/CameraData.h"
#include <queue>
#include "Device/FrameBuffer.h"
#include <RenderCommands.h>
#include "Graphics/Texture.h"
#include <Dementia.h>

#if ME_ENABLE_RENDERDOC
#include <Debug/RenderDocManager.h>
#endif
#include <Debug/DebugDrawer.h>
#include <Utils/CommandCache.h>

class ImGuiRenderer;

namespace Moonlight { class DynamicSky; }

struct RendererCreationSettings
{
	void* WindowPtr = nullptr;
	Vector2 InitialSize = Vector2(1280.f, 720.f);
};

enum class ViewportMode : uint8_t
{
	Game = 0,
	Editor,
	Count
};

class BGFXRenderer
{
	static constexpr bgfx::ViewId kClearView = 0;
public:
	BGFXRenderer() = default;

	void Create(const RendererCreationSettings& settings);
	void Destroy();

	void BeginFrame(const Vector2& mousePosition, uint8_t mouseButton, int32_t scroll, Vector2 outputSize, int inputChar, bgfx::ViewId viewId);
	void Render(Moonlight::CameraData& EditorCamera);

	void RenderCameraView(Moonlight::CameraData& camera, bgfx::ViewId id);

	void RenderSingleMesh(bgfx::ViewId id, const Moonlight::MeshCommand& mesh, uint64_t state);

	void WindowResized(const Vector2& newSize);

	// Cameras
	CommandCache<Moonlight::CameraData>& GetCameraCache();

	// Meshes
	CommandCache<Moonlight::MeshCommand>& GetMeshCache();
	void UpdateMeshMatrix(unsigned int Id, glm::mat4& matrix);
	void ClearMeshes();
	SharedPtr<Moonlight::DynamicSky> GetSky();

private:
	Vector2 PreviousSize;
	Vector2 CurrentSize;

	ImGuiRenderer* ImGuiRender = nullptr;

	CommandCache<Moonlight::CameraData> m_cameraCache;
	CommandCache<Moonlight::MeshCommand> m_meshCache;

	Moonlight::FrameBuffer* EditorCameraBuffer = nullptr;

	bgfx::VertexBufferHandle m_vbh;
	bgfx::IndexBufferHandle m_ibh;
	bgfx::ProgramHandle UIProgram;
	bgfx::UniformHandle s_texDiffuse;
	bgfx::UniformHandle s_texNormal;
	bgfx::UniformHandle s_texAlpha;
	bgfx::UniformHandle s_texUI;
	bgfx::UniformHandle s_ambient;
	bgfx::UniformHandle s_sunDirection;
	bgfx::UniformHandle s_sunDiffuse;
	bx::Vec3 m_ambient;
	int32_t m_pt;
	int64_t m_timeOffset;
	Moonlight::CameraData DummyCameraData;
	SharedPtr<Moonlight::Texture> m_defaultOpacityTexture;
	SharedPtr<Moonlight::DynamicSky> m_dynamicSky;

	UniquePtr<DebugDrawer> m_debugDraw;
#if ME_ENABLE_RENDERDOC
		RenderDocManager* RenderDoc;
#endif
};
