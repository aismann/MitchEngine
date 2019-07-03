#include "D3D12Device.h"
#include "Utils/DirectXHelper.h"

#if ME_DIRECTX

#include <DirectXColors.h>
#include <CommonStates.h>
#include "Graphics/MeshData.h"
#include <d3dcompiler.h>
#include "Utils/StringUtils.h"

using namespace D2D1;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
#if ME_PLATFORM_UWP
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Popups;
using namespace Platform;
#endif
using namespace std;

namespace Moonlight
{
	std::unique_ptr<DirectX::CommonStates> CommonStatesHelper;

	// Constructor for DeviceResources.
	D3D12Device::D3D12Device()
		: IDevice()
		, m_screenViewport()
		, m_d3dFeatureLevel(D3D_FEATURE_LEVEL_11_1)
		, m_logicalSize()
		, m_d3dRenderTargetSize()
		, m_outputSize(1280.f, 720.f)
		, m_deviceNotify(nullptr)
	{
		CreateFactories();
		CreateDeviceResources();
		CreateWindowSizeDependentResources();

		//MessageDialog Dialog("The renderer is currently under construction.", "Sorry!");
		//Dialog.ShowAsync();
	}

	// Configures resources that don't depend on the Direct3D device.
	void D3D12Device::CreateFactories()
	{
		// Initialize Direct2D resources.
		D2D1_FACTORY_OPTIONS options;
		ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
		// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

		// Initialize the DirectWrite Factory.
		DX::ThrowIfFailed(
			DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory3),
				&m_dwriteFactory
			)
		);

		// Initialize the Windows Imaging Component (WIC) Factory.
		DX::ThrowIfFailed(
			CoCreateInstance(
				CLSID_WICImagingFactory2,
				nullptr,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARGS(&m_wicFactory)
			)
		);
	}

	// Configures the Direct3D device, and stores handles to it and the device context.
	void D3D12Device::CreateDeviceResources()
	{
		// This flag adds support for surfaces with a different color channel ordering
		// than the API default. It is required for compatibility with Direct2D.
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
		if (DX::SdkLayersAvailable())
		{
			// If the project is in a debug build, enable debugging via SDK Layers with this flag.
			creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}
#endif

		// This array defines the set of DirectX hardware feature levels this app will support.
		// Note the ordering should be preserved.
		// Don't forget to declare your application's minimum required feature level in its
		// description.  All applications are assumed to support 9.1 unless otherwise stated.
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		// Create the Direct3D 11 API device object and a corresponding context.
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;

		HRESULT hr = D3D11CreateDevice(
			nullptr,					// Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the hardware graphics driver.
			0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			creationFlags,				// Set debug and Direct2D compatibility flags.
			featureLevels,				// List of feature levels this app can support.
			ARRAYSIZE(featureLevels),	// Size of the list above.
			D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&device,					// Returns the Direct3D device created.
			&m_d3dFeatureLevel,			// Returns feature level of device created.
			&context					// Returns the device immediate context.
		);
		DX::ThrowIfFailed(hr);
		if (FAILED(hr))
		{
			// If the initialization fails, fall back to the WARP device.
			// For more information on WARP, see: 
			// https://go.microsoft.com/fwlink/?LinkId=286690
			DX::ThrowIfFailed(
				D3D11CreateDevice(
					nullptr,
					D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
					0,
					creationFlags,
					featureLevels,
					ARRAYSIZE(featureLevels),
					D3D11_SDK_VERSION,
					&device,
					&m_d3dFeatureLevel,
					&context
				)
			);
		}
		// Store pointers to the Direct3D 11.3 API device and immediate context.
		DX::ThrowIfFailed(
			device.As(&m_d3dDevice)
		);

		DX::ThrowIfFailed(
			context.As(&m_d3dContext)
		);

		// Create the Direct2D device object and a corresponding context.
		ComPtr<IDXGIDevice3> dxgiDevice;
		DX::ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
		);

		D3D11_RASTERIZER_DESC wfdesc;
		ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
		wfdesc.FillMode = D3D11_FILL_SOLID;
		wfdesc.CullMode = D3D11_CULL_FRONT;
		hr = device->CreateRasterizerState(&wfdesc, &FrontFaceCulled);
		GetD3DDeviceContext()->RSSetState(FrontFaceCulled);

		wfdesc.FillMode = D3D11_FILL_WIREFRAME;
		hr = device->CreateRasterizerState(&wfdesc, &WireFrame);

		GetD3DDeviceContext()->RSSetState(WireFrame);

		CommonStatesHelper = std::make_unique<DirectX::CommonStates>(m_d3dDevice.Get());

		TransparentBlendState = CommonStatesHelper->NonPremultiplied();

		for (MSAASamples = MaxMSAASamples; MSAASamples > 1; MSAASamples /= 2)
		{
			UINT ColorQuality;
			UINT DepthStencilQuality;
			m_d3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R16G16B16A16_FLOAT, MSAASamples, &ColorQuality);
			m_d3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, MSAASamples, &DepthStencilQuality);
			if (ColorQuality > 0 && DepthStencilQuality > 0)
			{
				break;
			}
		}
	}

	Microsoft::WRL::ComPtr<ID3DBlob> D3D12Device::CompileShader(const Path& FileName, const std::string& EntryPoint, const std::string& Profile)
	{
		UINT Flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		Flags |= D3DCOMPILE_DEBUG;
		Flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> Shader;
		Microsoft::WRL::ComPtr<ID3DBlob> ErrorBlob;

		DX::ThrowIfFailed(D3DCompileFromFile(StringUtils::ToWString(FileName.FullPath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), Profile.c_str(), Flags, 0, &Shader, &ErrorBlob));

		return Shader;
	}

	Moonlight::ShaderProgram D3D12Device::CreateShaderProgram(const Microsoft::WRL::ComPtr<ID3DBlob>& vsBytecode, const Microsoft::WRL::ComPtr<ID3DBlob>& psBytecode, const std::vector<D3D11_INPUT_ELEMENT_DESC>* inputLayoutDesc) const
	{
		ShaderProgram Program;

		if (FAILED(m_d3dDevice->CreateVertexShader(vsBytecode->GetBufferPointer(), vsBytecode->GetBufferSize(), nullptr, &Program.VertexShader))) {
			throw std::runtime_error("Failed to create vertex shader from compiled bytecode");
		}
		if (FAILED(m_d3dDevice->CreatePixelShader(psBytecode->GetBufferPointer(), psBytecode->GetBufferSize(), nullptr, &Program.PixelShader))) {
			throw std::runtime_error("Failed to create pixel shader from compiled bytecode");
		}

		if (inputLayoutDesc) {
			if (FAILED(m_d3dDevice->CreateInputLayout(inputLayoutDesc->data(), (UINT)inputLayoutDesc->size(), vsBytecode->GetBufferPointer(), vsBytecode->GetBufferSize(), &Program.InputLayout))) {
				throw std::runtime_error("Failed to create shader program input layout");
			}
		}
		return Program;
	}

	Moonlight::ComputeProgram D3D12Device::CreateComputeProgram(const Microsoft::WRL::ComPtr<ID3DBlob>& csBytecode) const
	{
		ComputeProgram program;
		DX::ThrowIfFailed(m_d3dDevice->CreateComputeShader(csBytecode->GetBufferPointer(), csBytecode->GetBufferSize(), nullptr, &program.ComputeShader));
		return program;
	}

	Microsoft::WRL::ComPtr<ID3D11SamplerState> D3D12Device::CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) const
	{
		D3D11_SAMPLER_DESC Desc = {};
		Desc.Filter = filter;
		Desc.AddressU = addressMode;
		Desc.AddressV = addressMode;
		Desc.AddressW = addressMode;
		Desc.MaxAnisotropy = (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
		Desc.MinLOD = 0;
		Desc.MaxLOD = D3D11_FLOAT32_MAX;

		ComPtr<ID3D11SamplerState> samplerState;

		DX::ThrowIfFailed(m_d3dDevice->CreateSamplerState(&Desc, &samplerState));

		return samplerState;
	}

	void D3D12Device::SetOutputSize(Vector2 NewSize)
	{
		if (NewSize != m_outputSize)
		{
			m_outputSize = NewSize;
			CreateWindowSizeDependentResources();
		}
	}

	void D3D12Device::InitD2DScreenTexture()
	{
		//Create the vertex buffer
		Vertex v[] =
		{
			// Front Face
			Vertex{{-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
			Vertex{{-1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},
			Vertex{{1.0f,  1.0f, -1.0f}, {1.0f,  1.0f, -1.0f}, {1.0f, 0.0f }},
			Vertex{{1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f }},
		};

		DWORD indices[] = {
			// Front Face
			0,  1,  2,
			0,  2,  3,
		};

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;

		iinitData.pSysMem = indices;
		GetD3DDevice()->CreateBuffer(&indexBufferDesc, &iinitData, &d2dIndexBuffer);


		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = v;
		DX::ThrowIfFailed(GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &d2dVertBuffer));

	}

	// These resources need to be recreated every time the window size is changed.
	void D3D12Device::CreateWindowSizeDependentResources()
	{
		UpdateRenderTargetSize();

		m_d3dRenderTargetSize = m_outputSize;

		if (m_outputSize == Vector2())
		{
			return;
		}
		// Clear the previous window size specific context.
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		m_d3dRenderTargetView = nullptr;

		m_d3dDepthStencilView = nullptr;
		m_d3dContext->Flush1(D3D11_CONTEXT_TYPE_ALL, nullptr);


		if (m_swapChain != nullptr)
		{
			// If the swap chain already exists, resize it.
			HRESULT hr = m_swapChain->ResizeBuffers(
				2, // Double-buffered swap chain.
				lround(m_d3dRenderTargetSize.X()),
				lround(m_d3dRenderTargetSize.Y()),
				DXGI_FORMAT_B8G8R8A8_UNORM,
				0
			);

			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
				// If the device was removed for any reason, a new device and swap chain will need to be created.
				HandleDeviceLost();

				// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
				// and correctly set up the new device.
				return;
			}
			else
			{
				DX::ThrowIfFailed(hr);
			}
		}
		else
		{
			// Otherwise, create a new one using the same adapter as the existing Direct3D device.
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

			swapChainDesc.Width = lround(m_d3dRenderTargetSize.X());		// Match the size of the window.
			swapChainDesc.Height = lround(m_d3dRenderTargetSize.Y());
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				// This is the most common swap chain format.
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1;								// Don't use multi-sampling.
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2;									// Use double-buffering to minimize latency.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// All Windows Store apps must use (DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL) this SwapEffect.
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			
			//swapChainDesc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
			//swapChainDesc.Windowed = TRUE;
			//swapChainDesc.Scaling = DXGI_SCALING_NONE;
			//swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

			// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
			ComPtr<IDXGIDevice3> dxgiDevice;
			DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

			ComPtr<IDXGIAdapter> dxgiAdapter;
			DX::ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));

			ComPtr<IDXGIFactory4> dxgiFactory;
			DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

			DX::ThrowIfFailed(m_d3dDevice->GetDeviceRemovedReason());
			
			ComPtr<IDXGISwapChain1> swapChain;
#if ME_PLATFORM_UWP
			m_window = CoreWindow::GetForCurrentThread();
			DX::ThrowIfFailed(
				dxgiFactory->CreateSwapChainForCoreWindow(
					m_d3dDevice.Get(),
					reinterpret_cast<IUnknown*>(m_window.Get()),
					&swapChainDesc,
					nullptr,
					&swapChain
				)
			);
#elif ME_PLATFORM_WIN64
			DX::ThrowIfFailed(
				dxgiFactory->CreateSwapChainForHwnd(
					m_d3dDevice.Get(),
					GetActiveWindow(),
					&swapChainDesc,
					nullptr,
					nullptr,
					&swapChain
				)
			);
			m_window = GetActiveWindow();
#endif

			DX::ThrowIfFailed(swapChain.As(&m_swapChain));

			// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
			// ensures that the application will only render after each VSync, minimizing power consumption.
			DX::ThrowIfFailed(
				dxgiDevice->SetMaximumFrameLatency(0)
			);
		}

		DX::ThrowIfFailed(m_swapChain->SetRotation(DXGI_MODE_ROTATION_IDENTITY));
		// Create a render target view of the swap chain back buffer.
		ComPtr<ID3D11Texture2D> backBuffer;
		DX::ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));

		DX::ThrowIfFailed(
			m_d3dDevice->CreateRenderTargetView(
				backBuffer.Get(),
				nullptr,
				&m_d3dRenderTargetView
			)
		);

		// Create a depth stencil view for use with 3D rendering if needed.
		CD3D11_TEXTURE2D_DESC1 depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			lround(m_d3dRenderTargetSize.X()),
			lround(m_d3dRenderTargetSize.Y()),
			1, // This depth stencil view has only one texture.
			1, // Use a single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
		);

		ComPtr<ID3D11Texture2D1> depthStencil;
		DX::ThrowIfFailed(
			m_d3dDevice->CreateTexture2D1(
				&depthStencilDesc,
				nullptr,
				&depthStencil
			)
		);

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateDepthStencilView(
				depthStencil.Get(),
				&depthStencilViewDesc,
				&m_d3dDepthStencilView
			)
		);

		// Set the 3D rendering viewport to target the entire window.
		m_screenViewport = CD3D11_VIEWPORT(
			0.0f,
			0.0f,
			m_d3dRenderTargetSize.X(),
			m_d3dRenderTargetSize.Y()
		);

		m_d3dContext->RSSetViewports(1, &m_screenViewport);

		InitD2DScreenTexture();
	}


	// Determine the dimensions of the render target and whether it will be scaled down.
	void D3D12Device::UpdateRenderTargetSize()
	{
		// Prevent zero size DirectX content from being created.
#if !ME_EDITOR
		m_outputSize.SetX(static_cast<float>(std::fmax(static_cast<int>(m_logicalSize.X()), 1)));
		m_outputSize.SetY(static_cast<float>(std::fmax(static_cast<int>(m_logicalSize.Y()), 1)));
#endif
	}

	FrameBuffer* D3D12Device::CreateFrameBuffer(UINT Width, UINT Height, UINT Samples, DXGI_FORMAT ColorFormat, DXGI_FORMAT DepthStencilFormat) const
	{
		FrameBuffer* NewBuffer = new FrameBuffer();
		NewBuffer->Width = Width;
		NewBuffer->Height = Height;
		NewBuffer->Samples = Samples;

		D3D11_TEXTURE2D_DESC Desc = {};
		Desc.Width = Width;
		Desc.Height = Height;
		Desc.MipLevels = 1;
		Desc.ArraySize = 1;
		Desc.SampleDesc.Count = Samples;

		if (ColorFormat != DXGI_FORMAT_UNKNOWN)
		{
			Desc.Format = ColorFormat;
			Desc.BindFlags = D3D11_BIND_RENDER_TARGET;
			if (Samples <= 1)
			{
				Desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
			}

			DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&Desc, nullptr, &NewBuffer->ColorTexture));

			DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&Desc, nullptr, &NewBuffer->NormalTexture));

			DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&Desc, nullptr, &NewBuffer->SpecularTexture));

			DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&Desc, nullptr, &NewBuffer->FinalTexture));

			D3D11_RENDER_TARGET_VIEW_DESC RenderView = {};
			RenderView.Format = Desc.Format;
			RenderView.ViewDimension = (Samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(NewBuffer->NormalTexture.Get(), &RenderView, &NewBuffer->NormalRenderTargetView));

			if (Samples <= 1)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC ShaderView = {};
				ShaderView.Format = Desc.Format;
				ShaderView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				ShaderView.Texture2D.MostDetailedMip = 0;
				ShaderView.Texture2D.MipLevels = 1;

				DX::ThrowIfFailed(m_d3dDevice->CreateShaderResourceView(NewBuffer->NormalTexture.Get(), &ShaderView, &NewBuffer->NormalShaderResourceView));
			}

			RenderView.Format = Desc.Format;
			RenderView.ViewDimension = (Samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(NewBuffer->ColorTexture.Get(), &RenderView, &NewBuffer->ColorRenderTargetView));

			if (Samples <= 1)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC ShaderView = {};
				ShaderView.Format = Desc.Format;
				ShaderView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				ShaderView.Texture2D.MostDetailedMip = 0;
				ShaderView.Texture2D.MipLevels = 1;

				DX::ThrowIfFailed(m_d3dDevice->CreateShaderResourceView(NewBuffer->ColorTexture.Get(), &ShaderView, &NewBuffer->ColorShaderResourceView));
			}

			DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(NewBuffer->SpecularTexture.Get(), &RenderView, &NewBuffer->SpecularRenderTargetView));

			if (Samples <= 1)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC ShaderView = {};
				ShaderView.Format = Desc.Format;
				ShaderView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				ShaderView.Texture2D.MostDetailedMip = 0;
				ShaderView.Texture2D.MipLevels = 1;

				DX::ThrowIfFailed(m_d3dDevice->CreateShaderResourceView(NewBuffer->SpecularTexture.Get(), &ShaderView, &NewBuffer->SpecularShaderResourceView));
			}

			RenderView.Format = Desc.Format;
			RenderView.ViewDimension = (Samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(NewBuffer->FinalTexture.Get(), &RenderView, &NewBuffer->RenderTargetView));

			if (Samples <= 1)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC ShaderView = {};
				ShaderView.Format = Desc.Format;
				ShaderView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				ShaderView.Texture2D.MostDetailedMip = 0;
				ShaderView.Texture2D.MipLevels = 1;

				DX::ThrowIfFailed(m_d3dDevice->CreateShaderResourceView(NewBuffer->FinalTexture.Get(), &ShaderView, &NewBuffer->ShaderResourceView));
			}
		}

		if (DepthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			Desc.Format = DepthStencilFormat;
			Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&Desc, nullptr, &NewBuffer->DepthStencilTexture));

			D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilView = {};
			DepthStencilView.Format = Desc.Format;
			DepthStencilView.ViewDimension = (Samples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
			DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(NewBuffer->DepthStencilTexture.Get(), &DepthStencilView, &NewBuffer->DepthStencilView));
		}
		
		return NewBuffer;
	}

	// This method is called in the event handler for the SizeChanged event.
	void D3D12Device::SetLogicalSize(Vector2 logicalSize)
	{
		if (m_logicalSize != logicalSize)
		{
			m_logicalSize = logicalSize;
			CreateWindowSizeDependentResources();
		}
	}

	// This method is called in the event handler for the DisplayContentsInvalidated event.
	void D3D12Device::ValidateDevice()
	{
		// The D3D Device is no longer valid if the default adapter changed since the device
		// was created or if the device has been removed.

		// First, get the information for the default adapter from when the device was created.

		ComPtr<IDXGIDevice3> dxgiDevice;
		DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

		ComPtr<IDXGIAdapter> deviceAdapter;
		DX::ThrowIfFailed(dxgiDevice->GetAdapter(&deviceAdapter));

		ComPtr<IDXGIFactory4> deviceFactory;
		DX::ThrowIfFailed(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory)));

		ComPtr<IDXGIAdapter1> previousDefaultAdapter;
		DX::ThrowIfFailed(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter));

		DXGI_ADAPTER_DESC1 previousDesc;
		DX::ThrowIfFailed(previousDefaultAdapter->GetDesc1(&previousDesc));

		// Next, get the information for the current default adapter.

		ComPtr<IDXGIFactory4> currentFactory;
		DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)));

		ComPtr<IDXGIAdapter1> currentDefaultAdapter;
		DX::ThrowIfFailed(currentFactory->EnumAdapters1(0, &currentDefaultAdapter));

		DXGI_ADAPTER_DESC1 currentDesc;
		DX::ThrowIfFailed(currentDefaultAdapter->GetDesc1(&currentDesc));

		// If the adapter LUIDs don't match, or if the device reports that it has been removed,
		// a new D3D device must be created.

		if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
			previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
			FAILED(m_d3dDevice->GetDeviceRemovedReason()))
		{
			// Release references to resources related to the old device.
			dxgiDevice = nullptr;
			deviceAdapter = nullptr;
			deviceFactory = nullptr;
			previousDefaultAdapter = nullptr;

			// Create a new device and swap chain.
			HandleDeviceLost();
		}
	}

	// Recreate all device resources and set them back to the current state.
	void D3D12Device::HandleDeviceLost()
	{
		GetD3DDevice()->Release();
		m_swapChain = nullptr;

		if (m_deviceNotify != nullptr)
		{
			m_deviceNotify->OnDeviceLost();
		}

		CreateDeviceResources();
		CreateWindowSizeDependentResources();

		if (m_deviceNotify != nullptr)
		{
			m_deviceNotify->OnDeviceRestored();
		}
	}

	// Register our DeviceNotify to be informed on device lost and creation.
	void D3D12Device::RegisterDeviceNotify(IDeviceNotify* deviceNotify)
	{
		m_deviceNotify = deviceNotify;
	}

	// Call this method when the app suspends. It provides a hint to the driver that the app 
	// is entering an idle state and that temporary buffers can be reclaimed for use by other apps.
	void D3D12Device::Trim()
	{
		ComPtr<IDXGIDevice3> dxgiDevice;
		m_d3dDevice.As(&dxgiDevice);

		dxgiDevice->Trim();
	}

	void D3D12Device::WindowSizeChanged(const Vector2& NewSize)
	{
		m_logicalSize = NewSize;
		CreateWindowSizeDependentResources();
		//m_swapChain->ResizeBuffers(0, NewSize.X(), NewSize.Y(), DXGI_FORMAT_UNKNOWN, 0);
	}

	const int D3D12Device::GetMSAASamples() const
	{
		return MSAASamples;
	}

	void D3D12Device::ResetCullingMode() const
	{
		GetD3DDeviceContext()->RSSetState(FrontFaceCulled);
	}

}
#endif