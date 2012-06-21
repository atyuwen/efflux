#ifndef _D3D_APP_HPP_INCLUDED_
#define _D3D_APP_HPP_INCLUDED_

#include "hr_timer.hpp"
#include "post_process.hpp"

class D3DApp
{
	struct Vertex
	{
		float3 pos;
		float2 tex;
	};

	struct ViewParameters
	{
		D3DXMATRIX view_matrix;
		D3DXMATRIX proj_matrix;
	};

public:
	D3DApp();
	virtual ~D3DApp();

public:
	bool Initialize(HINSTANCE hinstance, int width, int height);
	int Run();
	void Destroy();

	int GetWidth() const;
	int GetHeight() const;

public:
	static D3DApp* GetApp();
	static ID3D11Device* GetD3D11Device();
	static ID3D11DeviceContext* GetD3D11DeviceContext();
	static HRTimer* GetTimer();

public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	void RegisterTimerEvents();
	void TimerEventsProc(int cnt, const tstring& tag);

private:
	bool InitializeWindow();
	bool InitializeD3D();
	bool InitializeScene();
	bool InitializeShaders();

	void UpdateScene(float delta_time);
	void RenderScene();
	
	void SetViewport(int width, int height, ID3D11DepthStencilView* dsv);
	void UpdateNoiseTexture(float delta_time);
	void UpdateParticleTexture(float delta_time);
	void UpdateSpawnTexture(float delta_time);

private:
	HINSTANCE m_hinstance;
	int m_width;
	int m_height;
	HWND m_hwnd;
	HRTimer m_timer;

	IDXGISwapChain* m_swap_chain;
	ID3D11Device* m_d3d11_device;
	ID3D11DeviceContext* m_d3d11_device_context;

	ID3D11Texture2D* m_depthstencil_buffer;
	ID3D11DepthStencilView* m_depthstencil_view;

	ID3D11Texture2D* m_noise_ds_buffer;
	ID3D11DepthStencilView* m_noise_dsv;
	ID3D11Texture2D* m_particle_ds_buffer;
	ID3D11DepthStencilView* m_particle_dsv;
	ID3D11Texture2D* m_spawn_ds_buffer;
	ID3D11DepthStencilView* m_spawn_dsv;

	ID3D11Texture2D* m_back_buffer;
	ID3D11RenderTargetView* m_back_buffer_rtv;

	ID3D11Texture2D* m_noise_texture;
	ID3D11RenderTargetView* m_noise_rtv;
	ID3D11ShaderResourceView* m_noise_srv;
	float m_noise_update_timer;
	float m_noise_update_peroid;

	ID3D11Texture2D* m_particle_textures[2];
	ID3D11RenderTargetView* m_particle_rtvs[2];
	ID3D11ShaderResourceView* m_particles_srvs[2];
	int m_particle_read_idx;
	int m_particle_write_idx;

	ID3D11Texture2D* m_offscreen_textures[2];
	ID3D11RenderTargetView* m_offscreen_rtvs[2];
	ID3D11ShaderResourceView* m_offscreen_srvs[2];
	int m_offscreen_read_idx;
	int m_offscreen_write_idx;

	ID3D11Texture2D* m_spawn_texture;
	ID3D11RenderTargetView* m_spawn_rtv;
	ID3D11ShaderResourceView* m_spawn_srv;
	float2 m_emitter_pos;

	ID3D11Texture2D* m_opacity_shadow_map_textures[4];
	ID3D11RenderTargetView* m_opacity_shadow_map_rtvs[4];
	ID3D11ShaderResourceView* m_opacity_shadow_map_srvs[4];

	ID3D11Buffer* m_spawn_param_buffer;
	ID3D11Buffer* m_noise_param_buffer;
	ID3D11Buffer* m_particle_param_buffer;
	ID3D11Buffer* m_sort_param_buffer;

	PostProcessPtr m_copy_pp;
	PostProcessPtr m_noise_pp;
	PostProcessPtr m_particle_pp;
	PostProcessPtr m_spawn_pp;

	PostProcessPtr m_clean_pp;
	PostProcessPtr m_sort_pp;
	int m_sort_stage_no;
	int m_sort_step_no;

	PostProcessPtr m_blur_v_pp;
	PostProcessPtr m_blur_h_pp;
	PostProcessPtr m_back_pp;

	ID3D11Buffer* m_particle_vertex_buffer;
	ID3D11VertexShader* m_particle_vertex_shader;
	ID3D11GeometryShader* m_particle_geometry_shader;
	ID3D11PixelShader* m_particle_pixel_shader;
	ID3D11PixelShader* m_shadow_pixel_shader;
	ID3D11GeometryShader* m_shadow_geometry_shader;
	ID3D11InputLayout* m_particle_vertex_layout;
	ID3D11SamplerState* m_particle_sampler_state;
	ID3D11ShaderResourceView *m_sprite_texture;
	ID3D11BlendState *m_particle_blend_state;
	ID3D11BlendState *m_shadow_blend_state;
	ViewParameters m_particle_view;
	ViewParameters m_particle_shadow_view;
	ID3D11Buffer* m_particle_view_buffer;
	ID3D11Buffer* m_particle_shadow_view_buffer;
};

#endif  // _D3D_APP_HPP_INCLUDED_