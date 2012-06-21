Texture2D src_texture : register(t0);
SamplerState default_sampler: register(s0);

struct PS_Output
{
	float4 osm0 : SV_TARGET0;
	float4 osm1 : SV_TARGET1;
	float4 osm2 : SV_TARGET2;
	float4 osm3 : SV_TARGET3;
};

PS_Output ps_main(in float4 tc_z_life : TEXCOORD)
{
	PS_Output output;
	float a = src_texture.Sample(default_sampler, tc_z_life.xy).x;
	a = a * a * 0.1;
	float4 z = tc_z_life.zzzz;

	const float near = 1.40;
	const float delta = 0.2;
	float4 w0 = step(z, float4( 0,  1,  2,  3) * delta + near);
	float4 w1 = step(z, float4( 4,  5,  6,  7) * delta + near);
	float4 w2 = step(z, float4( 8,  9, 10, 11) * delta + near);
	float4 w3 = step(z, float4(12, 13, 14, 15) * delta + near);

	output.osm0 = w0 * a;
	output.osm1 = w1 * a;
	output.osm2 = w2 * a;
	output.osm3 = w3 * a;
	return output;
}
