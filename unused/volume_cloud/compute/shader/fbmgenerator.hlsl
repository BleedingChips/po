StructuredBuffer<float3> random_point : register(t0);
RWTexture3D<float> output : register(u0);
Texture3D input : register(t1);
SamplerState ss : register(s0);
cbuffer b0 : register(b0)
{
    float scale;
    float value_scale;
    float ValueMulity;
}


[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint point_size;
    uint stri;
    random_point.GetDimensions(point_size, stri);

    float3 size2;
    output.GetDimensions(size2.x, size2.y, size2.z);
    float3 Pos2 = DTid / (size2 - 1);
    float InputValue = input.SampleLevel(ss, Pos2, 0).x;
    Pos2 = Pos2 * scale;


    float MinLen = 10.0;
    
    for (uint count = 0; count < point_size; ++ count)
    {
        float3 Point = random_point[count];
        float3 Dis = Point - Pos2;
        Dis = frac(Dis);
        Dis = lerp(Dis, 1.0 - Dis, step(abs(1.0 - Dis), abs(Dis)));
        float3 CurrentPoint = (Dis + Pos2) / scale;
        float PointValue = input.SampleLevel(ss, CurrentPoint, 0).x;
        if (PointValue > 0.01)
        {
            MinLen = min(MinLen, length(Dis));
        }
    }

    output[DTid] = clamp(1.0 - MinLen / ValueMulity / scale, 0.0, 1.0) * value_scale + InputValue;
        



}