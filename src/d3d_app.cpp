#include "common.hpp"

#include <algorithm>
#include <numeric>
#include <boost/bind.hpp>
#include "ayw/vector.hpp"
#include "ayw/constant.hpp"
#include "d3d_app.hpp"
using namespace Ayw;
typedef Ayw::vector_4t<int> int4;

const tstring g_app_title		= TEXT("Efflux");

const int g_particle_tex_size	= 512;
const int g_noise_tex_size      = 256;
const int g_spawn_tex_size      = 64;
const int g_shadow_tex_size     = 256;
const int g_sort_step_per_frame = 4;

//////////////////////////////////////////////////////////////////////////
// static accessors
//////////////////////////////////////////////////////////////////////////
D3DApp g_app;

D3DApp* D3DApp::GetApp()
{
	return &g_app;
}

ID3D11Device* D3DApp::GetD3D11Device()
{
	return g_app.m_d3d11_device;
}

ID3D11DeviceContext* D3DApp::GetD3D11DeviceContext()
{
	return g_app.m_d3d11_device_context;
}

HRTimer* D3DApp::GetTimer()
{
	return &g_app.m_timer;
}

//////////////////////////////////////////////////////////////////////////
// constructor / destructor
//////////////////////////////////////////////////////////////////////////
D3DApp::D3DApp()
	: m_hinstance(NULL)
	, m_hwnd(NULL)
	, m_swap_chain(NULL)
	, m_d3d11_device(NULL)
	, m_d3d11_device_context(NULL)
	, m_depthstencil_buffer(NULL)
	, m_noise_ds_buffer(NULL)
	, m_noise_dsv(NULL)
	, m_particle_ds_buffer(NULL)
	, m_particle_dsv(NULL)
	, m_spawn_ds_buffer(NULL)
	, m_spawn_dsv(NULL)
	, m_back_buffer_rtv(NULL)
	, m_depthstencil_view(NULL)
	, m_back_buffer(NULL)
	, m_noise_texture(NULL)
	, m_noise_rtv(NULL)
	, m_noise_srv(NULL)
	, m_noise_param_buffer(NULL)
	, m_spawn_param_buffer(NULL)
	, m_particle_param_buffer(NULL)
	, m_sort_param_buffer(NULL)
	, m_particle_vertex_buffer(NULL)
	, m_particle_vertex_shader(NULL)
	, m_particle_geometry_shader(NULL)
	, m_shadow_geometry_shader(NULL)
	, m_particle_pixel_shader(NULL)
	, m_shadow_pixel_shader(NULL)
	, m_particle_vertex_layout(NULL)
	, m_particle_sampler_state(NULL)
	, m_particle_blend_state(NULL)
	, m_shadow_blend_state(NULL)
	, m_particle_view_buffer(NULL)
	, m_particle_shadow_view_buffer(NULL)
{
	ZeroMemory(m_particle_textures, sizeof(m_particle_textures));
	ZeroMemory(m_particles_srvs, sizeof(m_particles_srvs));
	ZeroMemory(m_particle_rtvs, sizeof(m_particle_rtvs));

	m_particle_read_idx = 0;
	m_particle_write_idx = 1;

	ZeroMemory(m_offscreen_textures, sizeof(m_offscreen_textures));
	ZeroMemory(m_offscreen_srvs, sizeof(m_offscreen_srvs));
	ZeroMemory(m_offscreen_rtvs, sizeof(m_offscreen_rtvs));

	m_offscreen_read_idx = 0;
	m_offscreen_write_idx = 1;

	ZeroMemory(m_opacity_shadow_map_textures, sizeof(m_opacity_shadow_map_textures));
	ZeroMemory(m_opacity_shadow_map_srvs, sizeof(m_opacity_shadow_map_srvs));
	ZeroMemory(m_opacity_shadow_map_rtvs, sizeof(m_opacity_shadow_map_rtvs));

	m_noise_update_timer = 0.0f;
	m_noise_update_peroid = 0.2f;

	m_sort_step_no = 1;
	m_sort_stage_no = 1;
}

D3DApp::~D3DApp()
{
	SAFE_RELEASE(m_particle_vertex_buffer);
	SAFE_RELEASE(m_particle_vertex_shader);
	SAFE_RELEASE(m_particle_geometry_shader);
	SAFE_RELEASE(m_shadow_geometry_shader);
	SAFE_RELEASE(m_particle_pixel_shader);
	SAFE_RELEASE(m_shadow_pixel_shader);
	SAFE_RELEASE(m_particle_vertex_layout);
	SAFE_RELEASE(m_particle_sampler_state);
	SAFE_RELEASE(m_particle_blend_state);
	SAFE_RELEASE(m_shadow_blend_state);
	SAFE_RELEASE(m_particle_view_buffer);
	SAFE_RELEASE(m_particle_shadow_view_buffer);

	SAFE_RELEASE(m_noise_param_buffer);
	SAFE_RELEASE(m_spawn_param_buffer);
	SAFE_RELEASE(m_particle_param_buffer);
	SAFE_RELEASE(m_sort_param_buffer);

	SAFE_RELEASE(m_noise_rtv);
	SAFE_RELEASE(m_noise_srv);
	SAFE_RELEASE(m_noise_texture);

	for (int i = 0; i != ARRAYSIZE(m_particle_textures); ++i)
	{
		SAFE_RELEASE(m_particle_rtvs[i]);
		SAFE_RELEASE(m_particles_srvs[i]);
		SAFE_RELEASE(m_particle_textures[i]);
	}

	for (int i = 0; i != ARRAYSIZE(m_offscreen_textures); ++i)
	{
		SAFE_RELEASE(m_offscreen_rtvs[i]);
		SAFE_RELEASE(m_offscreen_srvs[i]);
		SAFE_RELEASE(m_offscreen_textures[i]);
	}

	for (int i = 0; i != ARRAYSIZE(m_opacity_shadow_map_textures); ++i)
	{
		SAFE_RELEASE(m_opacity_shadow_map_rtvs[i]);
		SAFE_RELEASE(m_opacity_shadow_map_srvs[i]);
		SAFE_RELEASE(m_opacity_shadow_map_textures[i]);
	}

	SAFE_RELEASE(m_noise_ds_buffer);
	SAFE_RELEASE(m_noise_dsv);
	SAFE_RELEASE(m_particle_ds_buffer);
	SAFE_RELEASE(m_particle_dsv);
	SAFE_RELEASE(m_spawn_ds_buffer);
	SAFE_RELEASE(m_spawn_dsv);

	SAFE_RELEASE(m_depthstencil_buffer);
	SAFE_RELEASE(m_depthstencil_view);

	SAFE_RELEASE(m_back_buffer_rtv);
	SAFE_RELEASE(m_back_buffer);

	SAFE_RELEASE(m_d3d11_device_context);
	SAFE_RELEASE(m_d3d11_device);
	SAFE_RELEASE(m_swap_chain);
}

//////////////////////////////////////////////////////////////////////////
// public interfaces
//////////////////////////////////////////////////////////////////////////
bool D3DApp::Initialize(HINSTANCE hinstance, int width, int height)
{
	m_hinstance = hinstance;
	m_width = width;
	m_height = height;

	if (!InitializeWindow())
	{
		MessageBox(NULL, TEXT("Error creating window"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	if (!InitializeD3D())
	{
		MessageBox(NULL, TEXT("Error initializing D3D"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	if (!InitializeScene())
	{
		MessageBox(NULL, TEXT("Error initializing scene"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	if (!InitializeShaders())
	{
		MessageBox(NULL, TEXT("Error initializing shaders"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	RegisterTimerEvents();
	return true;
}

int D3DApp::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (m_timer.SyncTick(1.0f / 60.0f))
			{
				UpdateScene(m_timer.GetDeltaTime());
				RenderScene();
			}
			else
			{
				timeBeginPeriod(1);
				Sleep(1);
				timeEndPeriod(1);
			}
		}
	}
	return msg.wParam;
}

void D3DApp::Destroy()
{

}

int D3DApp::GetWidth() const
{
	return m_width;
}

int D3DApp::GetHeight() const
{
	return m_height;
}

//////////////////////////////////////////////////////////////////////////
// window procedure
//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK D3DApp::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

//////////////////////////////////////////////////////////////////////////
// timer events procedure
//////////////////////////////////////////////////////////////////////////
void D3DApp::RegisterTimerEvents()
{
	m_timer.AddEvent(1.0f, boost::bind(&D3DApp::TimerEventsProc, this, _1, _2), TEXT("update_fps"));
}

void D3DApp::TimerEventsProc(int cnt, const tstring& tag)
{
	if (tag == TEXT("update_fps"))
	{
		float delta_time = m_timer.GetDeltaTime();
		int fps = static_cast<int>(1.0f / std::max(delta_time, Ayw::c_eps));
		tstring title = g_app_title + TEXT("  FPS: ") + to_tstring(fps);
		SetWindowText(m_hwnd, title.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
// private subroutines
//////////////////////////////////////////////////////////////////////////
bool D3DApp::InitializeWindow()
{
	WNDCLASSEX wc;
	tstring wnd_class = TEXT("aywd3dapp");
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = D3DApp::WindowProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = wnd_class.c_str();
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	RECT rect = {0, 0, m_width, m_height};
	DWORD wnd_style = WS_OVERLAPPEDWINDOW & (~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX));
	AdjustWindowRect(&rect, wnd_style, FALSE);
	m_hwnd = CreateWindowEx(
		NULL,
		wnd_class.c_str(),
		g_app_title.c_str(),
		wnd_style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		m_hinstance,
		this);

	if (m_hwnd == NULL)
	{
		return false;
	}

	ShowWindow(m_hwnd, SW_SHOWNORMAL);
	UpdateWindow(m_hwnd);

	GetClientRect(m_hwnd, &rect);
	m_width = rect.right - rect.left;
	m_height = rect.bottom - rect.top;
	return true;
}

bool D3DApp::InitializeD3D()
{
	// describe our SwapChain Buffer
	DXGI_MODE_DESC mode_desc;
	ZeroMemory(&mode_desc, sizeof(DXGI_MODE_DESC));
	mode_desc.Width = m_width;
	mode_desc.Height = m_height;
	mode_desc.RefreshRate.Numerator = 60;
	mode_desc.RefreshRate.Denominator = 1;
	mode_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	mode_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	mode_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapchain_desc; 
	ZeroMemory(&swapchain_desc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapchain_desc.BufferDesc = mode_desc;
	swapchain_desc.SampleDesc.Count = 1;
	swapchain_desc.SampleDesc.Quality = 0;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.BufferCount = 1;
	swapchain_desc.OutputWindow = m_hwnd; 
	swapchain_desc.Windowed = TRUE;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// create DXGI factory to enumerate adapters
	IDXGIFactory1 *dxgi_factory;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgi_factory);	

	// use the first adapter	
	IDXGIAdapter1 *adapter;
	hr = dxgi_factory->EnumAdapters1(0, &adapter);
	dxgi_factory->Release();

	// create our Direct3D 11 Device and SwapChain
	hr = D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL, NULL,	D3D11_SDK_VERSION, &swapchain_desc, &m_swap_chain, &m_d3d11_device, NULL, &m_d3d11_device_context);

	// release the Adapter interface
	adapter->Release();

	// create our BackBuffer and Render Target
	hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_back_buffer);
	hr = m_d3d11_device->CreateRenderTargetView(m_back_buffer, NULL, &m_back_buffer_rtv);

	// describe our Depth/Stencil Buffer
	CD3D11_TEXTURE2D_DESC depthstencil_desc(DXGI_FORMAT_D24_UNORM_S8_UINT, m_width,  m_height);
	depthstencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthstencil_desc.MipLevels = 1;
	m_d3d11_device->CreateTexture2D(&depthstencil_desc, NULL, &m_depthstencil_buffer);
	m_d3d11_device->CreateDepthStencilView(m_depthstencil_buffer, NULL, &m_depthstencil_view);

	CD3D11_TEXTURE2D_DESC noise_ds_desc(DXGI_FORMAT_D24_UNORM_S8_UINT, g_noise_tex_size,  g_noise_tex_size);
	noise_ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	noise_ds_desc.MipLevels = 1;
	m_d3d11_device->CreateTexture2D(&noise_ds_desc, NULL, &m_noise_ds_buffer);
	m_d3d11_device->CreateDepthStencilView(m_noise_ds_buffer, NULL, &m_noise_dsv);

	CD3D11_TEXTURE2D_DESC particle_ds_desc(DXGI_FORMAT_D24_UNORM_S8_UINT, g_particle_tex_size,  g_particle_tex_size);
	particle_ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	particle_ds_desc.MipLevels = 1;
	m_d3d11_device->CreateTexture2D(&particle_ds_desc, NULL, &m_particle_ds_buffer);
	m_d3d11_device->CreateDepthStencilView(m_particle_ds_buffer, NULL, &m_particle_dsv);

	CD3D11_TEXTURE2D_DESC spawn_ds_desc(DXGI_FORMAT_D24_UNORM_S8_UINT, g_spawn_tex_size,  g_spawn_tex_size);
	spawn_ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	spawn_ds_desc.MipLevels = 1;
	m_d3d11_device->CreateTexture2D(&spawn_ds_desc, NULL, &m_spawn_ds_buffer);
	m_d3d11_device->CreateDepthStencilView(m_spawn_ds_buffer, NULL, &m_spawn_dsv);

	// set render target views and depth stencil view
	m_d3d11_device_context->OMSetRenderTargets(1, &m_back_buffer_rtv, m_depthstencil_view);

	// create noise texture
	CD3D11_TEXTURE2D_DESC noise_tex_desc(DXGI_FORMAT_R8G8B8A8_UNORM, g_noise_tex_size, g_noise_tex_size);
	noise_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	noise_tex_desc.MipLevels = 1;
	m_d3d11_device->CreateTexture2D(&noise_tex_desc, NULL, &m_noise_texture);
	m_d3d11_device->CreateShaderResourceView(m_noise_texture, NULL, &m_noise_srv);
	m_d3d11_device->CreateRenderTargetView(m_noise_texture, NULL, &m_noise_rtv);

	// create particle textures
	CD3D11_TEXTURE2D_DESC particle_tex_desc(DXGI_FORMAT_R16G16B16A16_FLOAT, g_particle_tex_size, g_particle_tex_size);
	particle_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	particle_tex_desc.MipLevels = 1;
	for (int i = 0; i != ARRAYSIZE(m_particle_textures); ++i)
	{
		m_d3d11_device->CreateTexture2D(&particle_tex_desc, NULL, &m_particle_textures[i]);
		m_d3d11_device->CreateShaderResourceView(m_particle_textures[i], NULL, &m_particles_srvs[i]);
		m_d3d11_device->CreateRenderTargetView(m_particle_textures[i], NULL, &m_particle_rtvs[i]);
	}

	// create spawn texture
	CD3D11_TEXTURE2D_DESC spawn_tex_desc(DXGI_FORMAT_R16G16B16A16_FLOAT, g_spawn_tex_size, g_spawn_tex_size);
	spawn_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	spawn_tex_desc.MipLevels = 1;
	m_d3d11_device->CreateTexture2D(&spawn_tex_desc, NULL, &m_spawn_texture);
	m_d3d11_device->CreateShaderResourceView(m_spawn_texture, NULL, &m_spawn_srv);
	m_d3d11_device->CreateRenderTargetView(m_spawn_texture, NULL, &m_spawn_rtv);

	// create off-screen textures
	CD3D11_TEXTURE2D_DESC offscreen_tex_desc(DXGI_FORMAT_R16G16B16A16_FLOAT, m_width, m_height);
	offscreen_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	offscreen_tex_desc.MipLevels = 1;
	for (int i = 0; i != ARRAYSIZE(m_offscreen_textures); ++i)
	{
		m_d3d11_device->CreateTexture2D(&offscreen_tex_desc, NULL, &m_offscreen_textures[i]);
		m_d3d11_device->CreateShaderResourceView(m_offscreen_textures[i], NULL, &m_offscreen_srvs[i]);
		m_d3d11_device->CreateRenderTargetView(m_offscreen_textures[i], NULL, &m_offscreen_rtvs[i]);
	}

	// create shadow map textures
	CD3D11_TEXTURE2D_DESC shadow_map_tex_desc(DXGI_FORMAT_R16G16B16A16_FLOAT, g_shadow_tex_size, g_shadow_tex_size);
	shadow_map_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	shadow_map_tex_desc.MipLevels = 1;
	for (int i = 0; i != ARRAYSIZE(m_opacity_shadow_map_textures); ++i)
	{
		m_d3d11_device->CreateTexture2D(&shadow_map_tex_desc, NULL, &m_opacity_shadow_map_textures[i]);
		m_d3d11_device->CreateShaderResourceView(m_opacity_shadow_map_textures[i], NULL, &m_opacity_shadow_map_srvs[i]);
		m_d3d11_device->CreateRenderTargetView(m_opacity_shadow_map_textures[i], NULL, &m_opacity_shadow_map_rtvs[i]);
	}

	// create constant buffers
	CD3D11_BUFFER_DESC buffer_desc(sizeof(float4), D3D11_BIND_CONSTANT_BUFFER);
	hr = m_d3d11_device->CreateBuffer(&buffer_desc, NULL, &m_noise_param_buffer);
	hr = m_d3d11_device->CreateBuffer(&buffer_desc, NULL, &m_spawn_param_buffer);
	hr = m_d3d11_device->CreateBuffer(&buffer_desc, NULL, &m_particle_param_buffer);

	CD3D11_BUFFER_DESC sort_param_desc(sizeof(int4), D3D11_BIND_CONSTANT_BUFFER);
	hr = m_d3d11_device->CreateBuffer(&sort_param_desc, NULL, &m_sort_param_buffer);

	CD3D11_BUFFER_DESC view_param_buffer_desc(sizeof(ViewParameters), D3D11_BIND_CONSTANT_BUFFER);
	hr = m_d3d11_device->CreateBuffer(&view_param_buffer_desc, NULL, &m_particle_view_buffer);
	hr = m_d3d11_device->CreateBuffer(&view_param_buffer_desc, NULL, &m_particle_shadow_view_buffer);

	// create a texture from file
	D3DX11CreateShaderResourceViewFromFile(m_d3d11_device, TEXT("media/sprite.bmp"), NULL, NULL, &m_sprite_texture, &hr);

	return true;
}

bool D3DApp::InitializeScene()
{
	// init particle vertices
	Vertex* vertices = new Vertex[g_particle_tex_size * g_particle_tex_size];

	for (int j = 0; j != g_particle_tex_size; ++j)
	{
		for (int i = 0; i != g_particle_tex_size; ++i)
		{
			float u = static_cast<float>(i) / g_particle_tex_size;
			float v = static_cast<float>(j) / g_particle_tex_size;
			vertices[i + g_particle_tex_size * j].tex = float2(u, v);
		}
	}

	D3D11_BUFFER_DESC vertex_buffer_desc;
	ZeroMemory(&vertex_buffer_desc, sizeof(vertex_buffer_desc));
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = sizeof(Vertex) * g_particle_tex_size * g_particle_tex_size;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;
	vertex_buffer_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertices_data; 
	ZeroMemory(&vertices_data, sizeof(vertices_data));
	vertices_data.pSysMem = vertices;
	D3DApp::GetD3D11Device()->CreateBuffer(&vertex_buffer_desc, &vertices_data, &m_particle_vertex_buffer);
	delete[] vertices;

	// init particle vertex shader
	ID3DBlob *vertex_shader_buffer = NULL;
	D3DX11CompileFromFile(
		TEXT("fx/particle_vs.hlsl"),
		NULL,
		NULL,
		"vs_main",
		"vs_4_0",
		D3D10_SHADER_DEBUG,
		0,
		NULL,
		&vertex_shader_buffer,
		NULL,
		NULL);

	D3DApp::GetD3D11Device()->CreateVertexShader(
		vertex_shader_buffer->GetBufferPointer(),
		vertex_shader_buffer->GetBufferSize(),
		NULL,
		&m_particle_vertex_shader);

	// create particle vertex layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	UINT num_elements = ARRAYSIZE(layout);
	m_d3d11_device->CreateInputLayout(
		layout,
		num_elements,
		vertex_shader_buffer->GetBufferPointer(),
		vertex_shader_buffer->GetBufferSize(),
		&m_particle_vertex_layout);

	SAFE_RELEASE(vertex_shader_buffer);

	// create particle sampler state
	CD3D11_SAMPLER_DESC sampler_desc(D3D11_DEFAULT);
	sampler_desc.Filter  = D3D11_FILTER_MIN_MAG_MIP_POINT;
	m_d3d11_device->CreateSamplerState(&sampler_desc, &m_particle_sampler_state);

	// create particle blend state
	D3D11_BLEND_DESC particle_blend_desc;
	D3D11_RENDER_TARGET_BLEND_DESC target_particle_blend_desc;
	target_particle_blend_desc.BlendEnable = true;
	target_particle_blend_desc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	target_particle_blend_desc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	target_particle_blend_desc.BlendOp = D3D11_BLEND_OP_ADD;
	target_particle_blend_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
	target_particle_blend_desc.DestBlendAlpha = D3D11_BLEND_ZERO;
	target_particle_blend_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	target_particle_blend_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	particle_blend_desc.AlphaToCoverageEnable = false;
	particle_blend_desc.IndependentBlendEnable = false;
	particle_blend_desc.RenderTarget[0] = target_particle_blend_desc;
	m_d3d11_device->CreateBlendState(&particle_blend_desc, &m_particle_blend_state);

	// create shadow blend state
	D3D11_BLEND_DESC shadow_blend_desc;
	D3D11_RENDER_TARGET_BLEND_DESC target_shadow_blend_desc;
	target_shadow_blend_desc.BlendEnable = true;
	target_shadow_blend_desc.SrcBlend = D3D11_BLEND_ONE;
	target_shadow_blend_desc.DestBlend = D3D11_BLEND_ONE;
	target_shadow_blend_desc.BlendOp = D3D11_BLEND_OP_ADD;
	target_shadow_blend_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
	target_shadow_blend_desc.DestBlendAlpha = D3D11_BLEND_ONE;
	target_shadow_blend_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	target_shadow_blend_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	shadow_blend_desc.AlphaToCoverageEnable = false;
	shadow_blend_desc.IndependentBlendEnable = false;
	shadow_blend_desc.RenderTarget[0] = target_shadow_blend_desc;
	m_d3d11_device->CreateBlendState(&shadow_blend_desc, &m_shadow_blend_state);

	// init particle geometry shader
	ID3DBlob *geometry_shader_buffer = NULL;
	ID3DBlob* error_buffer = NULL;
	D3DX11CompileFromFile(
		TEXT("fx/particle_gs.hlsl"),
		NULL,
		NULL,
		"gs_main",
		"gs_4_0",
		D3D10_SHADER_DEBUG,
		0,
		NULL,
		&geometry_shader_buffer,
		&error_buffer,
		NULL);

	D3DApp::GetD3D11Device()->CreateGeometryShader(
		geometry_shader_buffer->GetBufferPointer(),
		geometry_shader_buffer->GetBufferSize(),
		NULL,
		&m_particle_geometry_shader);

	SAFE_RELEASE(geometry_shader_buffer);

	// init shadow geometry shader
	{
		ID3DBlob *geometry_shader_buffer = NULL;
		D3DX11CompileFromFile(
			TEXT("fx/shadow_gs.hlsl"),
			NULL,
			NULL,
			"gs_main",
			"gs_4_0",
			D3D10_SHADER_DEBUG,
			0,
			NULL,
			&geometry_shader_buffer,
			NULL,
			NULL);

		D3DApp::GetD3D11Device()->CreateGeometryShader(
			geometry_shader_buffer->GetBufferPointer(),
			geometry_shader_buffer->GetBufferSize(),
			NULL,
			&m_shadow_geometry_shader);

		SAFE_RELEASE(geometry_shader_buffer);
	}

	// init particle pixel shader
	ID3DBlob *pixel_shader_buffer = NULL;
	D3DX11CompileFromFile(
		TEXT("fx/particle_ps.hlsl"),
		NULL,
		NULL,
		"ps_main",
		"ps_4_0",
		D3D10_SHADER_DEBUG,
		0,
		NULL,
		&pixel_shader_buffer,
		NULL,
		NULL);

	D3DApp::GetD3D11Device()->CreatePixelShader(
		pixel_shader_buffer->GetBufferPointer(),
		pixel_shader_buffer->GetBufferSize(),
		NULL,
		&m_particle_pixel_shader);

	SAFE_RELEASE(pixel_shader_buffer);

	// init shadow pixel shader
	ID3DBlob *shadow_pixel_shader_buffer = NULL;
	D3DX11CompileFromFile(
		TEXT("fx/shadow_ps.hlsl"),
		NULL,
		NULL,
		"ps_main",
		"ps_4_0",
		D3D10_SHADER_DEBUG,
		0,
		NULL,
		&shadow_pixel_shader_buffer,
		NULL,
		NULL);

	D3DApp::GetD3D11Device()->CreatePixelShader(
		shadow_pixel_shader_buffer->GetBufferPointer(),
		shadow_pixel_shader_buffer->GetBufferSize(),
		NULL,
		&m_shadow_pixel_shader);

	SAFE_RELEASE(shadow_pixel_shader_buffer);

	// init view parameters
	D3DXVECTOR3 eye(0, 0, -1.732f);
	D3DXVECTOR3 at(0, 0, 0);
	D3DXVECTOR3 up(0, 1, 0);
	D3DXMatrixLookAtLH(&m_particle_view.view_matrix, &eye, &at, &up);
	D3DXMatrixPerspectiveFovLH(&m_particle_view.proj_matrix, Ayw::c_pi / 3, static_cast<float>(m_width) / m_height,  0.1f, 10.0f);
	m_d3d11_device_context->UpdateSubresource(m_particle_view_buffer, 0, NULL, &m_particle_view, 0, 0);

	// init shadow view parameters
	D3DXVECTOR3 seye(-1.5f, 2.5f, -0.5f);
	D3DXMatrixLookAtLH(&m_particle_shadow_view.view_matrix, &seye, &at, &up);
	D3DXMatrixPerspectiveFovLH(&m_particle_shadow_view.proj_matrix, Ayw::c_pi / 3, 1.0f,  0.1f, 10.0f);
	m_d3d11_device_context->UpdateSubresource(m_particle_shadow_view_buffer, 0, NULL, &m_particle_shadow_view, 0, 0);

	return true;
}

bool D3DApp::InitializeShaders()
{
	m_noise_pp = PostProcessPtr(new PostProcess);
	if (!m_noise_pp->LoadPixelShaderFromFile(TEXT("pp_noise.hlsl"), TEXT("ps_main")))
	{
		return false;
	}

	m_particle_pp = PostProcessPtr(new PostProcess);
	if (!m_particle_pp->LoadPixelShaderFromFile(TEXT("pp_particle.hlsl"), TEXT("ps_main")))
	{
		return false;
	}

	m_spawn_pp = PostProcessPtr(new PostProcess);
	if (!m_spawn_pp->LoadPixelShaderFromFile(TEXT("pp_spawn.hlsl"), TEXT("ps_main")))
	{
		return false;
	}

	m_copy_pp = PostProcessPtr(new PostProcess);
	if (!m_copy_pp->LoadPixelShaderFromFile(TEXT("pp_copy.hlsl"), TEXT("ps_main")))
	{
		return false;
	}

	m_blur_h_pp = PostProcessPtr(new PostProcess);
	if (!m_blur_h_pp->LoadPixelShaderFromFile(TEXT("pp_blur.hlsl"), TEXT("gaussian_horizon")))
	{
		return false;
	}

	m_blur_v_pp = PostProcessPtr(new PostProcess);
	if (!m_blur_v_pp->LoadPixelShaderFromFile(TEXT("pp_blur.hlsl"), TEXT("gaussian_vertical")))
	{
		return false;
	}

	m_clean_pp = PostProcessPtr(new PostProcess);
	if (!m_clean_pp->LoadPixelShaderFromFile(TEXT("pp_sort.hlsl"), TEXT("clean")))
	{
		return false;
	}

	m_sort_pp = PostProcessPtr(new PostProcess);
	if (!m_sort_pp->LoadPixelShaderFromFile(TEXT("pp_sort.hlsl"), TEXT("sort")))
	{
		return false;
	}

	m_back_pp = PostProcessPtr(new PostProcess);
	if (!m_back_pp->LoadPixelShaderFromFile(TEXT("pp_back.hlsl"), TEXT("ps_main")))
	{
		MessageBoxf(m_back_pp->GetErrorMessage());
		return false;
	}

	return true;
}

void D3DApp::UpdateScene(float delta_time)
{
	UpdateSpawnTexture(delta_time);
	UpdateNoiseTexture(delta_time);
	UpdateParticleTexture(delta_time);
}

void D3DApp::RenderScene()
{
	// draw background
	SetViewport(m_width, m_height, m_depthstencil_view);
	m_d3d11_device_context->ClearRenderTargetView(m_back_buffer_rtv, float4(0, 0, 0, 1).ptr());
	m_d3d11_device_context->ClearDepthStencilView(m_depthstencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 255);
	m_back_pp->InputPin(0, m_opacity_shadow_map_srvs[3]);
	m_back_pp->OutputPin(0, m_back_buffer_rtv);
	m_back_pp->SetParameters(0, m_particle_shadow_view_buffer);
	m_back_pp->Apply();

	// set up particle vertices
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_d3d11_device_context->IASetVertexBuffers(0, 1, &m_particle_vertex_buffer, &stride, &offset);
	m_d3d11_device_context->IASetInputLayout(m_particle_vertex_layout);
	m_d3d11_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// opacity shadow map
	SetViewport(g_shadow_tex_size, g_shadow_tex_size, m_noise_dsv);
	ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {NULL};
	rtvs[0] = m_opacity_shadow_map_rtvs[0];
	rtvs[1] = m_opacity_shadow_map_rtvs[1];
	rtvs[2] = m_opacity_shadow_map_rtvs[2];
	rtvs[3] = m_opacity_shadow_map_rtvs[3];
	m_d3d11_device_context->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, NULL);
	m_d3d11_device_context->OMSetBlendState(m_shadow_blend_state, NULL, 0xFFFFFFFF);
	m_d3d11_device_context->ClearRenderTargetView(rtvs[0], float4(0, 0, 0, 0).ptr());
	m_d3d11_device_context->ClearRenderTargetView(rtvs[1], float4(0, 0, 0, 0).ptr());
	m_d3d11_device_context->ClearRenderTargetView(rtvs[2], float4(0, 0, 0, 0).ptr());
	m_d3d11_device_context->ClearRenderTargetView(rtvs[3], float4(0, 0, 0, 0).ptr());

	m_d3d11_device_context->VSSetShaderResources(0, 1, &m_particles_srvs[m_particle_read_idx]);
	m_d3d11_device_context->VSSetSamplers(0, 1, &m_particle_sampler_state);
	m_d3d11_device_context->VSSetShader(m_particle_vertex_shader, NULL, 0);
	m_d3d11_device_context->GSSetShader(m_shadow_geometry_shader, NULL, 0);
	m_d3d11_device_context->GSSetConstantBuffers(0, 1, &m_particle_shadow_view_buffer);
	m_d3d11_device_context->PSSetShader(m_shadow_pixel_shader, NULL, 0);
	m_d3d11_device_context->PSSetShaderResources(0, 1, &m_sprite_texture);
	m_d3d11_device_context->Draw(g_particle_tex_size * g_particle_tex_size, 0);

	// shading particles
	SetViewport(m_width, m_height, m_depthstencil_view);
	ZeroMemory(rtvs, sizeof(rtvs));
	rtvs[0] = m_back_buffer_rtv;
	m_d3d11_device_context->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, NULL);
	m_d3d11_device_context->OMSetBlendState(m_particle_blend_state, NULL, 0xFFFFFFFF);
	m_d3d11_device_context->VSSetShaderResources(0, 1, &m_particles_srvs[m_particle_read_idx]);
	m_d3d11_device_context->VSSetSamplers(0, 1, &m_particle_sampler_state);
	m_d3d11_device_context->VSSetShader(m_particle_vertex_shader, NULL, 0);
	m_d3d11_device_context->GSSetShader(m_particle_geometry_shader, NULL, 0);
	m_d3d11_device_context->GSSetConstantBuffers(0, 1, &m_particle_view_buffer);
	m_d3d11_device_context->GSSetConstantBuffers(1, 1, &m_particle_shadow_view_buffer);
	m_d3d11_device_context->GSSetShaderResources(0, 4, &m_opacity_shadow_map_srvs[0]);
	m_d3d11_device_context->PSSetShader(m_particle_pixel_shader, NULL, 0);
	m_d3d11_device_context->PSSetShaderResources(0, 1, &m_sprite_texture);
	m_d3d11_device_context->Draw(g_particle_tex_size * g_particle_tex_size, 0);

	m_swap_chain->Present(0, 0);
}

void D3DApp::SetViewport(int width, int height, ID3D11DepthStencilView* dsv)
{
	D3D11_VIEWPORT view_port;
	view_port.Width = static_cast<float>(width);
	view_port.Height = static_cast<float>(height);
	view_port.MinDepth = 0.0f;
	view_port.MaxDepth = 1.0f;
	view_port.TopLeftX = 0;
	view_port.TopLeftY = 0;
	m_d3d11_device_context->RSSetViewports(1, &view_port);
	m_d3d11_device_context->OMSetRenderTargets(0, NULL, dsv);
}

void D3DApp::UpdateNoiseTexture(float delta_time)
{
	if (m_noise_update_timer > 0)
	{
		m_noise_update_timer -= delta_time;
		return;
	}

	float4 param(0, 0, 0, m_timer.GetTime());
	m_d3d11_device_context->UpdateSubresource(m_noise_param_buffer, 0, NULL, &param, 0, 0);
	m_particle_pp->SetParameters(0, m_noise_param_buffer);

	SetViewport(g_noise_tex_size, g_noise_tex_size, m_noise_dsv);
	m_noise_pp->OutputPin(0, m_noise_rtv);
	m_noise_pp->Apply();
	m_noise_update_timer = m_noise_update_peroid;
}

void D3DApp::UpdateParticleTexture(float delta_time)
{
	SetViewport(g_particle_tex_size, g_particle_tex_size, m_particle_dsv);

	float4 param(static_cast<float>(g_particle_tex_size), static_cast<float>(g_noise_tex_size), m_timer.GetTime(), delta_time);
	m_d3d11_device_context->UpdateSubresource(m_particle_param_buffer, 0, NULL, &param, 0, 0);
	m_particle_pp->SetParameters(0, m_particle_param_buffer);

	m_particle_pp->InputPin(0, m_particles_srvs[m_particle_read_idx]);
	m_particle_pp->InputPin(1, m_spawn_srv);
	m_particle_pp->InputPin(2, m_noise_srv);

	m_particle_pp->OutputPin(0, m_particle_rtvs[m_particle_write_idx]);
	m_particle_pp->Apply();
	std::swap(m_particle_read_idx, m_particle_write_idx);

	// sort particles
	for (int i = 0; i != g_sort_step_per_frame; ++i)
	{
		PostProcessPtr sort_pp = (m_sort_step_no == m_sort_stage_no) ? m_clean_pp : m_sort_pp;	
		int4 sort_param(m_sort_step_no, m_sort_stage_no, 2 * m_sort_stage_no, g_particle_tex_size);
		m_d3d11_device_context->UpdateSubresource(m_sort_param_buffer, 0, NULL, &sort_param, 0, 0);
		sort_pp->SetParameters(0, m_sort_param_buffer);
		sort_pp->InputPin(0, m_particles_srvs[m_particle_read_idx]);
		sort_pp->OutputPin(0, m_particle_rtvs[m_particle_write_idx]);
		sort_pp->Apply();

		m_sort_step_no /= 2;
		if (m_sort_step_no == 0)
		{
			m_sort_stage_no *= 2;
			if (m_sort_stage_no >= g_particle_tex_size * g_particle_tex_size) m_sort_stage_no = 1;
			m_sort_step_no = m_sort_stage_no;
		}
	}
}

void D3DApp::UpdateSpawnTexture(float delta_time)
{
	SetViewport(g_spawn_tex_size, g_spawn_tex_size, m_spawn_dsv);

	POINT mouse_pos;
	GetCursorPos(&mouse_pos);
	ScreenToClient(m_hwnd, &mouse_pos);
	mouse_pos.x = std::max<int>(0, std::min<int>(m_width, mouse_pos.x));
	mouse_pos.y = std::max<int>(0, std::min<int>(m_height, mouse_pos.y));
	float2 pos(static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y));
	pos = pos / float2(static_cast<float>(m_width), static_cast<float>(m_height));
	pos = pos * 2.0f - float2(1.0f, 1.0f);
	pos.y = - pos.y;

	float2 dist = pos - m_emitter_pos;
	float2 dir(0.0f, 0.0f);
	if (dist.length_sqr() > c_eps) dir = dist.normalize();
	m_emitter_pos += dir * 0.8f * delta_time;

	float4 param(m_emitter_pos.x * static_cast<float>(m_width) / m_height, m_emitter_pos.y, 0, m_timer.GetTime());
	m_d3d11_device_context->UpdateSubresource(m_spawn_param_buffer, 0, NULL, &param, 0, 0);
	m_spawn_pp->SetParameters(0, m_spawn_param_buffer);

	m_spawn_pp->OutputPin(0, m_spawn_rtv);
	m_spawn_pp->Apply();
}
