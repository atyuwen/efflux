#include "snoise.hlsl"

cbuffer Paramerters
{
	// w = elapsed time
	float4 param;
};

float fBm_noise(float2 x)
{
  float y = snoise(x);
  y += snoise(2 * x) * 0.5;
  y += snoise(4 * x) * 0.25;
  y += snoise(8 * x) * 0.125;
  return (y / 1.875) * 0.5 + 0.5;
}

float turbulent_noise(float2 x)
{
  float y = abs(snoise(x));
  y += abs(snoise(2 * x) * 0.5);
  y += abs(snoise(4 * x) * 0.25);
  y += abs(snoise(8 * x) * 0.125);
  return y / 1.875;
}

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
  float2 p = (in_tex * 2 - 1) * 2;
  p.x += sin(param.w * 0.103);
  p.y += cos(param.w * 0.079);

  float3 f = 0;
  f.x = fBm_noise(p);
  f.y = fBm_noise(p + 79.263);
  f.z = fBm_noise(p + 108.54);
  return float4(f, 1);
}
