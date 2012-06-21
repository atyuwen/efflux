Texture2D particle_texture : register(t0);
SamplerState default_sampler: register(s0);

void vs_main(in float3 in_pos : POSITION,
             in float2 in_tex : TEXCOORD,
			 out float life   : TEXCOORD,
			 out float4 pos   : SV_POSITION)
{
	float4 p = particle_texture.Load(int3(in_tex * 512, 0));
	if (p.w > 0)
	{
		pos = float4(p.xyz, 1);
	}
	else
	{
		pos = float4(1000, 1000, 1000, 1);
	}

	life = 1.0 - p.w / 6.0;
}
