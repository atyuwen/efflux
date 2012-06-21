Texture2D src_texture : register(t0);
SamplerState default_sampler: register(s0);

float4 ps_main(in float4 tc_z_life : TEXCOORD0, in float shadow : TEXCOORD1) : SV_TARGET
{
	tc_z_life.w = saturate(tc_z_life.w);
	float3 rgb = lerp(float3(0.8, 0.2, 0.8), float3(0.2, 0.4, 0.8), tc_z_life.w);
	rgb *= shadow;
	float a = src_texture.Sample(default_sampler, tc_z_life.xy).x;
	return float4(rgb, a * a * 0.1);
}
