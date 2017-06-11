cbuffer ff :register (b0)
{
	float da[92];
};

RWTexture2D<float2> outSurface : register(u0);


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

    float tr = (3.141592653 * 2.0 / 90);

	float2 po = DTid.xy / 1024.0 - 0.5;
    float2 nor = normalize(po);
    float r = acos(nor.y) / tr;
    int index = r;
    
    if(nor.x >= 0.0)
        index = 91 - index;

	index += 1;

    float2 p[4];
    for (int i = 0; i < 4; ++i)
    {
        p[i] = float2(sin((index - 1) * tr), cos((index - 1) * tr)) * (0.5 + da[i] / 4.0);
    }

    float2 ba = p[0] * pow(1.0 - r, 3.0) + p[1] * pow(1.0 - r, 2.0) * r + p[2] * pow(r, 2.0) * (1.0 - r) + p[3] * pow(r, 3.0);
    float f = 0.0;
    if (length(po) < length(ba))
        f = 1.0;
    outSurface[DTid.xy]= float2(f, 0.0);

}