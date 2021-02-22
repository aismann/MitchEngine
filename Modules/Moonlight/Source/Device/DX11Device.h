#pragma once
#include "IDevice.h"
//
//#ifdef ME_DIRECTX
//
//#include <d3d11_3.h>
//#include <d2d1_3.h>
//#include <dxgi1_4.h>
//#include <dwrite_3.h>
//#include <wincodec.h>
//#include <wrl/client.h>
//
//#if ME_PLATFORM_UWP
//#include <agile.h>
//#endif
//
//#include <memory>
//#include "Path.h"
//#include "FrameBuffer.h"
//#include "Graphics/ShaderStructures.h"
//#include <map>
//#include <string>
//#include "Pointers.h"
//
//namespace Moonlight
//{
//	class DX11Device
//		: public IDevice
//	{
//	public:
//		DX11Device();
//
//		// The size of the render target, in pixels.
//		Vector2 GetOutputSize() const { return m_outputSize; }
//		virtual void SetLogicalSize(Vector2 logicalSize);
//		void ValidateDevice();
//		void HandleDeviceLost();
//		void RegisterDeviceNotify(IDeviceNotify* deviceNotify);
//		void Trim();
//
//		virtual void WindowSizeChanged(const Vector2& NewSize) final;
//
//		const int GetMSAASamples() const;
//		void ResetCullingMode() const;
//		// The size of the render target, in dips.
//		Vector2					GetLogicalSize() const { return m_logicalSize; }
//
//		// D3D Accessors.
//		ID3D11Device3*				GetD3DDevice() const { return m_d3dDevice.Get(); }
//		ID3D11DeviceContext3*		GetD3DDeviceContext() const { return m_d3dContext.Get(); }
//		IDXGISwapChain3*			GetSwapChain() const { return m_swapChain.Get(); }
//		D3D_FEATURE_LEVEL			GetDeviceFeatureLevel() const { return m_d3dFeatureLevel; }
//		ID3D11RenderTargetView*	GetBackBufferRenderTargetView() const { return m_d3dRenderTargetView.Get(); }
//		//ID3D11DepthStencilView*		GetDepthStencilView() const { return m_d3dDepthStencilView.Get(); }
//		D3D11_VIEWPORT				GetScreenViewport() const { return m_screenViewport; }
//		Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const Path& FileName, const std::string& EntryPoint, const std::string& Profile);
//
//		ShaderProgram CreateShaderProgram(const std::string& FilePath, const Microsoft::WRL::ComPtr<ID3DBlob>& vsBytecode, const Microsoft::WRL::ComPtr<ID3DBlob>& psBytecode, const std::vector<D3D11_INPUT_ELEMENT_DESC>* inputLayoutDesc);
//
//		ComputeProgram CreateComputeProgram(const Microsoft::WRL::ComPtr<ID3DBlob>& csBytecode) const;
//
//		Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) const;
//
//		bool FindShader(const std::string& InPath, ShaderProgram& outShader);
//
//		// D2D Accessors.
//		IDWriteFactory3*			GetDWriteFactory() const { return m_dwriteFactory.Get(); }
//		IWICImagingFactory2*		GetWicImagingFactory() const { return m_wicFactory.Get(); }
//		void SetOutputSize(Vector2 NewSize);
//
//		void InitD2DScreenTexture();
//		FrameBuffer* CreateFrameBuffer(UINT Width, UINT Height, UINT Samples, DXGI_FORMAT ColorFormat, DXGI_FORMAT DepthStencilFormat) const;
//
//		virtual void CreateWindowSizeDependentResources() final;
//#if ME_PLATFORM_WIN64
//		HWND m_window;
//#endif
//		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_d3dRenderTargetView;
//		ID3D11BlendState* TransparentBlendState = nullptr;
//		Microsoft::WRL::ComPtr<ID3D11Buffer>		d2dVertBuffer;
//		Microsoft::WRL::ComPtr<ID3D11Buffer>		d2dIndexBuffer;
//		
//		ID3D11DepthStencilState* DepthStencilState;
//		ID3D11RasterizerState* FrontFaceCulled;
//	private:
//		virtual void CreateFactories() final;
//		virtual void CreateDeviceResources() final;
//		void UpdateRenderTargetSize();
//
//		std::map<std::string, ShaderProgram> m_shaderCache;
//
//		// Direct3D objects.
//		Microsoft::WRL::ComPtr<ID3D11Device3>			m_d3dDevice;
//		Microsoft::WRL::ComPtr<ID3D11DeviceContext3>	m_d3dContext;
//		Microsoft::WRL::ComPtr<IDXGISwapChain3>			m_swapChain;
//		ID3D11RasterizerState*							WireFrame;
//
//		// Direct3D rendering objects. Required for 3D.
//		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_d3dDepthStencilView;
//		D3D11_VIEWPORT									m_screenViewport;
//
//		// DirectWrite drawing components.
//		Microsoft::WRL::ComPtr<IDWriteFactory3>		m_dwriteFactory;
//		Microsoft::WRL::ComPtr<IWICImagingFactory2>	m_wicFactory;
//
//
//		// Cached reference to the Window.
//#if ME_PLATFORM_UWP
//		Platform::Agile<Windows::UI::Core::CoreWindow> m_window;
//#endif
//
//		// Cached device properties.
//		D3D_FEATURE_LEVEL								m_d3dFeatureLevel;
//		Vector2										m_logicalSize;
//		Vector2						m_d3dRenderTargetSize;
//		Vector2						m_outputSize;
//
//		// The IDeviceNotify can be held directly as it owns the DeviceResources.
//		IDeviceNotify* m_deviceNotify;
//		const int MaxMSAASamples = 16;
//		int MSAASamples = 1;
//	};
//}

//#endif