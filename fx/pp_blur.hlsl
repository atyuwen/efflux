Texture2D src_texture : register(t0);
SamplerState default_sampler: register(s0);

float4 gaussian_horizon(in float2 in_tex : TEXCOORD) : SV_TARGET
{
	float2 dim = float2(0, 0);
	src_texture.GetDimensions(dim.x, dim.y);
	float delta = 1 / dim.x;

	float4 s1 = src_texture.Sample(default_sampler, in_tex);
	float4 s2 = src_texture.Sample(default_sampler, in_tex + float2(delta, 0));
	float4 s3 = src_texture.Sample(default_sampler, in_tex - float2(delta, 0));
	float4 s4 = src_texture.Sample(default_sampler, in_tex + float2(2 * delta, 0));
	float4 s5 = src_texture.Sample(default_sampler, in_tex - float2(2 * delta, 0));

	return s1 * 0.375 + (s2 + s3) * 0.25 + (s4 + s5) * 0.0625;
}

float4 gaussian_vertical(in float2 in_tex : TEXCOORD) : SV_TARGET
{
	float2 dim = float2(0, 0);
	src_texture.GetDimensions(dim.x, dim.y);
	float delta = 1 / dim.y;

	float4 s1 = src_texture.Sample(default_sampler, in_tex);
	float4 s2 = src_texture.Sample(default_sampler, in_tex + float2(0, delta));
	float4 s3 = src_texture.Sample(default_sampler, in_tex - float2(0, delta));
	float4 s4 = src_texture.Sample(default_sampler, in_tex + float2(0, 2 * delta));
	float4 s5 = src_texture.Sample(default_sampler, in_tex - float2(0, 2 * delta));

	return s1 * 0.375 + (s2 + s3) * 0.25 + (s4 + s5) * 0.0625;
}

