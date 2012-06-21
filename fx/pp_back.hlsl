Texture2D osm_texture : register(t0);
SamplerState default_sampler: register(s0);

cbuffer ShadowTransformParameters : register(c0)
{
	matrix shadow_view_matrix;
	matrix shadow_proj_matrix;
};

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
	float2 p = in_tex * 2.0 - 1.0;
	float d = length(p - float2(0, 0.4));
	d = 0.8 - d * 0.3 + p.y * 0.1;
	float3 col = float3(d, d, d);

	in_tex.y -= 0.4;
	float4 q = float4(p.x / in_tex.y, 0, 1 / in_tex.y - 2, 1);
	float4 shadow_view_pos = mul(shadow_view_matrix, q);
	float4 shadow_proj_pos = mul(shadow_proj_matrix, shadow_view_pos);
	float2 stc = shadow_proj_pos.xy / shadow_proj_pos.w;
	stc = stc * 0.5 + 0.5;
	stc.y = 1 - stc.y;

	float density = osm_texture.Sample(default_sampler, stc).w;
	float shadow = 1 - density / (density + 100);
	col *= shadow;

	return float4(col, 1);
}
