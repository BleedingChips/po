
cbuffer B0 : register(b0)
{
    uint3 textue_size;
}

RWTexture3D<float> Out : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 poi = DTid / float3(textue_size - 1);
    float3 EdgeDetect = abs(poi - 0.5);
    float3 EdgeStep = step(0.3, EdgeDetect);
    EdgeDetect = EdgeDetect * EdgeStep;
    float3 EdgeTraget = 0.3 * EdgeStep;
    float Value = distance(EdgeTraget, EdgeDetect);
    float Factor = clamp(-5 * Value + 1, 0.0, 1.0);
    Out[DTid] = Factor * Factor;
}