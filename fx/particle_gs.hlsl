Texture2D osm0 : register(t0);
Texture2D osm1 : register(t1);
Texture2D osm2 : register(t2);
Texture2D osm3 : register(t3);
SamplerState default_sampler: register(s0);

cbuffer TransformParameters : register(c0)
{
	matrix view_matrix;
	matrix proj_matrix;
};

cbuffer ShadowTransformParameters : register(c1)
{
	matrix shadow_view_matrix;
	matrix shadow_proj_matrix;
};

static const float4 offsets[4] =
{
	float4(-1,  1,  0,  0),
	float4( 1,  1,  0,  0),
	float4(-1, -1,  0,  0),
	float4( 1, -1,  0,  0),
};

static const float2 texcoords[4] =
{
	float2(0, 1),
	float2(1, 1),
	float2(0, 0),
	float2(1, 0),
};

struct GS_Input
{
	float life : TEXCOORD;
	float4 pos : SV_POSITION;
};

struct GS_Output
{
	float4 tc_z_life: TEXCOORD0;
	float shadow: TEXCOORD1;
	float4 pos: SV_POSITION;
};

[maxvertexcount(4)]
void gs_main(point GS_Input input[1], inout TriangleStream<GS_Output> sprites)
{
	GS_Output output;

	float4 shadow_view_pos = mul(shadow_view_matrix, input[0].pos);
	float4 shadow_proj_pos = mul(shadow_proj_matrix, shadow_view_pos);
	float2 stc = shadow_proj_pos.xy / shadow_proj_pos.w;
	stc = stc * 0.5 + 0.5;
	stc.y = 1 - stc.y;

	const float near = 1.40;
	const float delta = 0.2;
	float4 w0 = max(1 - abs(shadow_view_pos.z - (float4( 0,  1,  2,  3) * delta + near)) / delta, 0);
	float4 w1 = max(1 - abs(shadow_view_pos.z - (float4( 4,  5,  6,  7) * delta + near)) / delta, 0);
	float4 w2 = max(1 - abs(shadow_view_pos.z - (float4( 8,  9, 10, 11) * delta + near)) / delta, 0);
	float4 w3 = max(1 - abs(shadow_view_pos.z - (float4(12, 13, 14, 15) * delta + near)) / delta, 0);

	float density = 0;
	density += dot(osm0.SampleLevel(default_sampler, stc, 0), w0);
	density += dot(osm1.SampleLevel(default_sampler, stc, 0), w1);
	density += dot(osm2.SampleLevel(default_sampler, stc, 0), w2);
	density += dot(osm3.SampleLevel(default_sampler, stc, 0), w3);
	output.shadow = 1 - density / (density + 10);

	float4 view_pos = mul(view_matrix, input[0].pos);
	for (int i = 0; i < 4; ++i)
	{
		output.pos = mul(proj_matrix, view_pos + offsets[i] * 0.01);
		output.tc_z_life.xy = texcoords[i];
		output.tc_z_life.z = view_pos.z;
		output.tc_z_life.w = input[0].life;
		sprites.Append(output);
	}
	sprites.RestartStrip();
}
