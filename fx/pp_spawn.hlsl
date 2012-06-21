#include "snoise.hlsl"

cbuffer Paramerters
{
	// xyz = emitter location
	// w = elapsed time
	float4 param;
};

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
	float2 p = in_tex * 2 - 1;

	float x = param.x + snoise(50 * p + param.w) * 0.05;
	float y = param.y + snoise(61 * p + param.w) * 0.05;
	float z = param.z + snoise(72 * p + param.w) * 0.1;
	float t = snoise(83 * p + param.w) * 6 + 8;

	return float4(x, y, z, t);
}
