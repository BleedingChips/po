static const uint n_point = 300;

cbuffer generator_patameter : register(b0)
{
    uint4 size;
    float step;
    float3 worley_noise_poin[n_point];
};

float3 count_position(uint2 input)
{
    float3 tem = float3(
    input.x % size.x,
    input.y % size.y,
    input.x / size.x + input.y / size.y * size.z
);
    return tem / float3(size.x - 1, size.y - 1, size.z * size.w - 1);
}


RWTexture2D<float4> out_texture : register(u0);
RWTexture2D<float4> out_texture1 : register(u1);

void compress(inout float i, inout float i2)
{
    float min_m = min(i, i2);
    i2 = max(i, i2);
    i = min_m;
}

void push_stack(in float d, inout float4 last)
{
    compress(last.w, d);
    compress(last.z, last.w);
    compress(last.y, last.z);
    compress(last.x, last.y);
}

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 max_dis = float4(1.0, 1.0, 1.0, 1.0);
    uint count = 0;
    float3 pos = count_position(DTid.xy);
    for (count = 0; count < n_point; ++count)
    {
        float dis = min(distance(pos, worley_noise_poin[count]) * step, 1.0);
        push_stack(dis, max_dis);
    }
    out_texture[DTid.xy] = max_dis;
    out_texture1[DTid.xy] = float4(1.0, 1.0, 1.0, 1.0);
    //float4(max_dis.x, max_dis.x, max_dis.x, max_dis.x);
}