
struct property
{
    uint2 texture_size;
    uint4 simulate_size;
    float3 cube_min;
    float3 cube_max;
};

cbuffer B0 : register(b0)
{
    property pro;
}

RWTexture2D<float> Out : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint3 tex_size = uint3(pro.simulate_size.x, pro.simulate_size.y, pro.simulate_size.z * pro.simulate_size.w);
    uint3 loc = uint3(
        DTid.x % pro.simulate_size.x,
        DTid.y % pro.simulate_size.y,
        DTid.y / pro.simulate_size.y * pro.simulate_size.z + DTid.x / pro.simulate_size.x
        );
    float3 loc_f = loc / float3(tex_size - 1);
    if (
        loc_f.x > pro.cube_min.x && loc_f.x < pro.cube_max.x &&
        loc_f.y > pro.cube_min.y && loc_f.y < pro.cube_max.y &&
        loc_f.z > pro.cube_min.z && loc_f.z < pro.cube_max.z
)
        Out[DTid.xy] = 1.0;
    else
        Out[DTid.xy] = 0.0;
}