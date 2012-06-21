Texture2D src_texture : register(t0);

cbuffer Paramerters
{
	// x = stepno      = [1, 2 1, 4 2 1, ...]
	// y = stageno     = [1, 2 2, 4 4 4, ...]
	// z = 2 * stageno = [2, 4 4, 8 8 8, ...]
	// w = tex size
	int4 param;
};

int2 get_coord2d(int coord1d, int width)
{
	int2 coord2d;
	coord2d.y = coord1d / width;
	coord2d.x = coord1d - coord2d.y * width;
	return coord2d;
}

float4 clean(in float4 pos : SV_POSITION) : SV_TARGET
{
	int2 p2d = pos.xy;
	int p1d = p2d.y * param.w + p2d.x;
	int parity = fmod(p1d / param.x, 2);

	int q1d = p1d + param.z - 1 - 2 * fmod(p1d, param.z);
	int2 q2d = get_coord2d(q1d, param.w);

	float4 vp = src_texture.Load(int3(p2d, 0));
	float4 vq = src_texture.Load(int3(q2d, 0));
	float4 vmin = (vp.z < vq.z) ? vp : vq;
	float4 vmax = (vp.z < vq.z) ? vq : vp;

	return (parity == 0) ? vmax : vmin;
}

float4 sort(in float4 pos : SV_POSITION) : SV_TARGET
{
	int2 p2d = pos.xy;
	int p1d = p2d.y * param.w + p2d.x;
	int parity = fmod(p1d / param.x, 2);
	int dir = parity * 2 - 1;
	int q1d = p1d + dir * param.x;
	int2 q2d = get_coord2d(q1d, param.w);

	float4 vp = src_texture.Load(int3(p2d, 0));
	float4 vq = src_texture.Load(int3(q2d, 0));
	float4 vmin = (vp.z < vq.z) ? vp : vq;
	float4 vmax = (vp.z < vq.z) ? vq : vp;

	return (parity == 0) ? vmax : vmin;
}
