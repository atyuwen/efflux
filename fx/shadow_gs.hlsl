cbuffer TransformParameters : register(c0)
{
	matrix view_matrix;
	matrix proj_matrix;
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
	float4 tc_z_life: TEXCOORD;
	float4 pos: SV_POSITION;
};

[maxvertexcount(4)]
void gs_main(point GS_Input input[1], inout TriangleStream<GS_Output> sprites)
{
	GS_Output output;
	float4 view_pos = mul(view_matrix, input[0].pos);
	for (int i = 0; i < 4; ++i)
	{
		output.pos = mul(proj_matrix, view_pos + offsets[i] * 0.03);
		output.tc_z_life.xy = texcoords[i];
		output.tc_z_life.z = view_pos.z;
		output.tc_z_life.w = input[0].life;
		sprites.Append(output);
	}
	sprites.RestartStrip();
}
