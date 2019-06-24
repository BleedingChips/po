#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "../../../../../include/po_dx11_defer_renderer/shader/include/build_in_placement_defer_inout.hlsli"
#include "../../../../../include/po_dx11_defer_renderer/shader/include/build_in_property_buffer_type.hlsli"

Texture3D volume_texture : register(t0);
SamplerState SS : register(s0);

cbuffer renderer_3d : register(b0)
{
    renderer_3d_buffer_t rb;
};

cbuffer transfer_3d : register(b1)
{
    transfer_3d_static_buffer_t ts;
};

cbuffer volume_state : register(b2)
{
    float3 light;
    float max_density;
    float3 min_local;
    float3 max_local;
}

float cal_noise(float3 world_position, float time)
{

    float3 poi = (world_position + float3(time / 10000.0, 0.0, 0.0));

    float resule = (
    perlin_noise(poi * 4, float3(1.0, 2334, 34)) * 0.5 +
    perlin_noise(poi * 8, float3(0.4, 0.56, 23)) * 0.25 +
    perlin_noise(poi * 16, float3(23445, 466, 23)) * 0.125 +
    perlin_noise(poi * 32, float3(23445, 466, 23)) * 0.125 
) *
  
     (volume_texture.Sample(SS, (poi + time / 10000.0 + 1.0) / 2.0).x);



    return resule * resule;
    
}

float4 main(placement_defer_vs_output inp) : SV_TARGET
{
    float4 eye_position = float4(0.0, 0.0, 0.0, 1.0);

    eye_position = mul(rb.screen_to_world, eye_position);


    float3 eye_world_dir = normalize(inp.world_position - eye_position.xyz / eye_position.w);
    //float4 eye_world_dirp = mul(rb.screen_to_world, float4(eye_world_dir, 0.0));
   // eye_world_dir = eye_world_dirp.xyz / eye_world_dirp.w;
    float3 eye_local_dir = normalize(mul(ts.world_to_local, float4(eye_world_dir, 0.0)).xyz);
    float3 size = (max_local - min_local);

    eye_local_dir = eye_local_dir / max(max(abs(eye_local_dir.x), abs(eye_local_dir.y)), abs(eye_local_dir.z)) * size / 64;
    float len = length(eye_local_dir);

    float time_shift = rb.time / 10000.0;

    float volume_rate = cal_noise(inp.local_position, rb.time);
    //-volume_texture.Sample(SS, inp.local_position / 2.0 + time_shift).x * 0.7;
    //volume_texture.Sample(SS, (inp.local_position - min_local) / size).x;
    float decay = 0.0;

    uint count = 0;
    for (count = 1; count < 64; ++count)
    {
        float3 poi = inp.local_position + eye_local_dir * count;
        if (
            poi.x < min_local.x || poi.x > max_local.y ||
            poi.y < min_local.y || poi.y > max_local.y ||
            poi.z < min_local.z || poi.z > max_local.z 
        )
        {
            decay = decay + volume_rate * max_density * len;
            break;
        }
            
        
        float volume_rage_curenmt = cal_noise(poi, rb.time);
        //-volume_texture.Sample(SS, poi / 2.0 + time_shift).x * 0.7;
        //volume_texture.Sample(SS, (poi - min_local) / size).x;
        decay = decay + (volume_rate + volume_rage_curenmt) / 2.0 * max_density * len;
        //float decay_current = exp(-(volume_rate + volume_rage_curenmt) / 2.0 * max_density * len);
       // decay = decay * decay_current;
        volume_rate = volume_rage_curenmt;
        
       
    }
    float e_decay = exp(-decay);
    float insv_decay = 2.0 - e_decay * e_decay;
    //return float4(1.0 - exp(-decay * 2.0), 1.0 - exp(-decay * 2.0), 1.0 - exp(-decay * 2.0), 1.0 - exp(-decay));
    return float4(e_decay, 1.0, 1.0, 1.0 - e_decay);
}