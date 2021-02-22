#pragma once
//#include <d3d11.h>
//#include <wrl/client.h>
//#include <Ultralight/platform/GPUDriver.h>
//#include <map>
//#include <vector>
//#include <memory>
//#include "UI/GPUDriverImpl.h"
//
//namespace Moonlight { struct ShaderProgram; }
//
//namespace Moonlight { class DX11Device; }
//
//using Microsoft::WRL::ComPtr;
//
//namespace ultralight {
//
//class GPUContextD3D11;
//
///**
// * GPUDriver implementation for Direct3D 11.
// */
//class GPUDriverD3D11 : public GPUDriverImpl {
//public:
//  GPUDriverD3D11(GPUContextD3D11* context, Moonlight::DX11Device* device);
//  virtual ~GPUDriverD3D11();
//
//  virtual void CreateTexture(uint32_t texture_id, Ref<Bitmap> bitmap) override;
//
//  virtual void UpdateTexture(uint32_t texture_id, Ref<Bitmap> bitmap) override;
//
//  virtual void BindTexture(uint8_t texture_unit, uint32_t texture_id) override;
//
//  virtual void DestroyTexture(uint32_t texture_id) override;
//
//  virtual void CreateRenderBuffer(uint32_t render_buffer_id, const RenderBuffer& buffer) override;
//
//  virtual void BindRenderBuffer(uint32_t render_buffer_id) override;
//
//  virtual void ClearRenderBuffer(uint32_t render_buffer_id) override;
//
//  virtual void DestroyRenderBuffer(uint32_t render_buffer_id) override;
//
//  virtual void CreateGeometry(uint32_t geometry_id,
//    const VertexBuffer& vertices,
//    const IndexBuffer& indices) override;
//
//  virtual void UpdateGeometry(uint32_t geometry_id,
//    const VertexBuffer& vertices,
//    const IndexBuffer& indices) override;
//
//  virtual void DrawGeometry(uint32_t geometry_id,
//    uint32_t indices_count,
//    uint32_t indices_offset,
//    const GPUState& state) override;
//
//  virtual void DestroyGeometry(uint32_t geometry_id) override;
//
//  virtual void UpdateCommandList(const CommandList& list) override;
//
//  virtual bool HasCommandsPending() override { return !command_list_.empty(); }
//
//
//  const char* name() override;
//
//
//  void BeginDrawing() override;
//
//
//  void EndDrawing() override;
//
//protected:
//  void LoadShaders();
//  void BindShader(uint8_t shader);
//  void BindVertexLayout(VertexBufferFormat format);
//  void BindGeometry(uint32_t id);
//  ComPtr<ID3D11SamplerState> GetSamplerState();
//  ComPtr<ID3D11Buffer> GetConstantBuffer();
//  void SetViewport(uint32_t width, uint32_t height);
//  void UpdateConstantBuffer(const GPUState& state);
//  Matrix ApplyProjection(const Matrix4x4& transform, float screen_width, float screen_height);
//
//  GPUContextD3D11* context_;
//  Moonlight::DX11Device* m_device;
//  ComPtr<ID3D11SamplerState> sampler_state_;
//  ComPtr<ID3D11Buffer> constant_buffer_;
//
//  struct GeometryEntry { VertexBufferFormat format; ComPtr<ID3D11Buffer> vertexBuffer; ComPtr<ID3D11Buffer> indexBuffer; };
//  typedef std::map<uint32_t, GeometryEntry> GeometryMap;
//  GeometryMap geometry_;
//
//  struct TextureEntry { 
//    ComPtr<ID3D11Texture2D> texture;
//    ComPtr<ID3D11ShaderResourceView> texture_srv;
//
//    // These members are only used when MSAA is enabled
//    bool is_msaa_render_target = false;
//    bool needs_resolve = false;
//    ComPtr<ID3D11Texture2D> resolve_texture;
//    ComPtr<ID3D11ShaderResourceView> resolve_texture_srv;
//  };
//
//  typedef std::map<uint32_t, TextureEntry> TextureMap;
//  TextureMap textures_;
//
//  struct RenderTargetEntry {
//    ComPtr<ID3D11RenderTargetView> render_target_view;
//    uint32_t render_target_texture_id;
//  };
//
//  typedef std::map<uint32_t, RenderTargetEntry> RenderTargetMap;
//  RenderTargetMap render_targets_;
//
//  std::map<ShaderType, Moonlight::ShaderProgram> shaders_;
//};
//
//}  // namespace ultralight
