Texture2D particle_texture  : register(t0);
Texture2D spawn_texture		: register(t1);
Texture2D noise_texture		: register(t2);

SamplerState default_sampler: register(s0);

cbuffer Paramerters
{
	// x = particle texture size
	// y = noise texture size
	// z = elapsed time
	// w = delta time
	float4 param;
};

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
	float4 particle = particle_texture.Sample(default_sampler, in_tex);
	
	if (particle.w > 0)
	{
		float3 p = particle.xyz * 0.5 + 0.5;
		float d = 1 / param.y;

		float fx = noise_texture.Sample(default_sampler, p.yz).x;
		float fy = noise_texture.Sample(default_sampler, p.xz).y;
		float fz = noise_texture.Sample(default_sampler, p.xy).z;

		float fxdy = noise_texture.Sample(default_sampler, p.yz + float2(d, 0)).x - fx;
	    float fxdz = noise_texture.Sample(default_sampler, p.yz + float2(0, d)).x - fx;

		float fydx = noise_texture.Sample(default_sampler, p.xz + float2(d, 0)).y - fy;
	    float fydz = noise_texture.Sample(default_sampler, p.xz + float2(0, d)).y - fy;

		float fzdx = noise_texture.Sample(default_sampler, p.xy + float2(d, 0)).z - fz;
	    float fzdy = noise_texture.Sample(default_sampler, p.xy + float2(0, d)).z - fz;

		float3 v = float3(fzdy - fydz, fxdz - fzdx, fydx  - fxdy) * param.y * 0.005;

		particle.xyz += v * param.w;
		particle.w -= param.w;
	}
	else
	{
		particle = spawn_texture.Sample(default_sampler, in_tex);
	}

	return particle;
}
