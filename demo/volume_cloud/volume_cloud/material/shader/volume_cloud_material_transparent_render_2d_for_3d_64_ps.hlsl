#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11_defer_renderer/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11_defer_renderer/shader/include/build_in_standard_input_type.hlsli"
cbuffer b0 : register(b0)
{
    property_rendder_2d_for_3d pp;
}
cbuffer b1 : register(b1)
{
    property_transfer mat;
}
cbuffer b2 : register(b2)
{
    property_screen ps;
}

Texture2D ten : register(t0);

Texture2D linearize_z : register(t1);
SamplerState ss;

/*
    ray_start_point_local：体表面的像素的局部坐标在进行归一化到[0,1]之间后的坐标。
    ray_normalize_local：在世界坐标下的计算的射线向量，单位化后转换到局部坐标系。
    max_density：体素里1.0所表示的浓度值。
    worley_size：2d Worley Noise 所表示的3d 噪声的大小。
    target_depth：要计算最终的射线长度
    random_seed：产生Perlin Noise的随机数种子
*/
float4 implement(
float3 ray_start_point_local,
float3 ray_local,
float3 min_width,
float3 max_width,
float max_density, 
uint4 worley_size, 
float target_depth,
float3 random_seed, 
float time,
float3 local_light,
float3 scale
);

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{
    //立方体在局部坐标系下的长宽高最大最小值
    float3 min_w = float3(-1.0, -1.0, -1.0);
    float3 max_w = float3(1.0, 1.0, 1.0);

    float2 screen_uv = (input.uv_screen / input.position_sv.w + 1.0) / 2.0;

    //世界坐标系下的射线向量，并单位化，升维
    float4 eye_ray_NOR = float4(normalize(input.position_world.xyz - ps.view_position), 0.0);

    //计算深度差
    float depth_length = max(linearize_z.Sample(ss, screen_uv).x - input.position_view.z, 0.0);

    //局部坐标系下的射线向量，降维，此时由于立方体本身的变换矩阵，所以不是单位化的
    eye_ray_NOR = mul(mat.world_to_local, eye_ray_NOR);

    output.color = //float4(linearize_z.Sample(ss, screen_uv).x / 1000, 0.0, 0.0, 1.0);
    //float4(input.uv_screen, 0.0, 1.0);
    implement(input.position_local.xyz / input.position_local.w, eye_ray_NOR.xyz, min_w, max_w, pp.density, uint4(128, 128, 16, 8), depth_length, float3(0.123, 0.456, 0.789), ps.time * 0.0001, float3(0.2, -1.0, 0.0), float3(2.0, 2.0, 2.0));
}

float4 implement(
float3 ray_start_point_local,
float3 ray_local,
float3 min_width,
float3 max_width,
float max_density,
uint4 worley_size,
float target_depth,
float3 random_seed,
float time,
float3 local_light,
float3 scale
)
{

#define COORDINATE_I3_TO_I3(out_I3, in_I3, worley_I4) { int4 worley = worley_I4;int3 width_3 = int3(worley.x - 1, worley.y - 1, worley.z * worley.w - 1) * in_I3;out_I3 = width_3;}

#define COORDINATE_I3_TO_I2(WorleyCoordinate_Out_I2, WorleyCoordinate_In_I3, WorleySize_I4) {int3 tem1 = WorleyCoordinate_In_I3;int4 size = WorleySize_I4;int3 noise_size = int3(size.x, size.y, size.z * size.w) - 2;uint3 tem2 = abs(tem1);tem1 = tem1 % (noise_size * 2);tem1 = noise_size - abs(noise_size - abs(tem1));WorleyCoordinate_Out_I2 = int2((tem1.z % size.z) * size.x + tem1.x,(tem1.z / size.z) * size.y + tem1.y);}

#define LINEAR_SAMPLE(Sample_Result_F4, Coordinate_F3, Sample_Texture_T, Worley_Size_I4) {float3 Coord = Coordinate_F3;int4 worley_size_INI = Worley_Size_I4;Coord = Coord * float3(worley_size_INI.x - 1, worley_size_INI.y - 1, worley_size_INI.z * worley_size_INI.w - 1);int3 Coord_floor = floor(Coord);float3 Rate = Coord - Coord_floor;int4 Size = Worley_Size_I4;int3 Size_3 = int3(Size.x - 1, Size.y - 1, Size.z * Size.w - 1);int2 TextureCoord;COORDINATE_I3_TO_I2(TextureCoord, Coord_floor, worley_size_INI);float4 tem = Sample_Texture_T[TextureCoord];COORDINATE_I3_TO_I2(TextureCoord, Coord_floor + int3(1, 0, 0), worley_size_INI);float4 tem2 = Sample_Texture_T[TextureCoord];tem = lerp(tem, tem2, Rate.x);COORDINATE_I3_TO_I2(TextureCoord, Coord_floor + int3(0, 1, 0), worley_size_INI);tem2 = Sample_Texture_T[TextureCoord];COORDINATE_I3_TO_I2(TextureCoord, Coord_floor + int3(1, 1, 0), worley_size_INI);float4 tem3 = Sample_Texture_T[TextureCoord];tem2 = lerp(tem2, tem3, Rate.x);tem = lerp(tem, tem2, Rate.y);COORDINATE_I3_TO_I2(TextureCoord, Coord_floor + int3(0, 0, 1), worley_size_INI);tem2 = Sample_Texture_T[TextureCoord];COORDINATE_I3_TO_I2(TextureCoord, Coord_floor + int3(1, 0, 1), worley_size_INI);tem3 = Sample_Texture_T[TextureCoord];tem2 = lerp(tem2, tem3, Rate.x);COORDINATE_I3_TO_I2(TextureCoord, Coord_floor + int3(0, 1, 1), worley_size_INI);tem3 = Sample_Texture_T[TextureCoord];COORDINATE_I3_TO_I2(TextureCoord, Coord_floor + int3(1, 1, 1), worley_size_INI);float4 tem4 = Sample_Texture_T[TextureCoord];tem3 = lerp(tem3, tem4, Rate.x);tem2 = lerp(tem2, tem3, Rate.y);tem = lerp(tem, tem2, Rate.z);Sample_Result_F4 = tem;}

#define HG(result, input_light, ray_light, g_function) {float3 il = normalize(input_light);float3 rl = normalize(ray_light); float g = g_function;float g2 = g * g;result = 0.5 * (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * dot(il, rl), 1.5);}

#define PERLIN_NOISE_RATE(output_F, input_F) {float t = input_F;output_F = (t * t * t * (t * (t * 6 - 15) + 10));}

#define RENDOM(output_F, input_F3) {float3 t = input_F3;output_F = frac(sin(dot(t.xyz, float3(12.9898, 78.233, 42.1897))) * 43758.5453);}

#define PERLIN_NOISE(output_F, base_location_F3, ran_seed_F3) { float3 bl = base_location_F3; float3 rs = ran_seed_F3; float3 o = floor(bl); float3 rate = bl - o; PERLIN_NOISE_RATE(rate.x, rate.x); PERLIN_NOISE_RATE(rate.y, rate.y);PERLIN_NOISE_RATE(rate.z, rate.z);float temporary1;RENDOM(temporary1, rs);float3 ran_seed = float3(temporary1, temporary1, temporary1);float3 rand_seed2 = rs * rs;float temporary2;RENDOM(temporary1, rs * (o) + rand_seed2);RENDOM(temporary2, rs * (o + float3(1.0, 0.0, 0.0)) + rand_seed2);float temporary3;temporary3 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x;RENDOM(temporary1, ran_seed * (o + float3(0.0, 1.0, 0.0)) + rand_seed2);RENDOM(temporary2, ran_seed * (o + float3(1.0, 1.0, 0.0)) + rand_seed2);temporary1 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x; float temporary4;temporary4 = temporary3 * (1.0 - rate.y) + temporary1 * rate.y;RENDOM(temporary1, ran_seed * (o + float3(0.0, 0.0, 1.0)) + rand_seed2);RENDOM(temporary2, ran_seed * (o + float3(1.0, 0.0, 1.0)) + rand_seed2);temporary3 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x;RENDOM(temporary1, ran_seed * (o + float3(0.0, 1.0, 1.0)) + rand_seed2);RENDOM(temporary2, ran_seed * (o + float3(1.0, 1.0, 1.0)) + rand_seed2);temporary1 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x;temporary3 = temporary3 * (1.0 - rate.y) + temporary1 * rate.y;output_F = temporary4 * (1.0 - rate.z) + temporary3 * rate.z;}

#define CALCULATE_NOISE(result_F, position_F3, texture_T, scale_F3, worley_size_I4, Random_seed1_F3, Random_seed2_F3, Random_seed3_F3, Random_seed4_F3, move_normal_F3, time_F) {float3 r1 = Random_seed1_F3;float3 r2 = Random_seed2_F3;float3 r3 = Random_seed3_F3;float3 r4 = Random_seed4_F3;float3 shift = normalize(move_normal_F3) * time_F;float3 poi_in = position_F3;float3 scale_in = scale_F3;float3 perlin_poi = position_F3 + shift;float re_tem;float re = 0;PERLIN_NOISE(re_tem, perlin_poi * 4.0, r1);re = re + re_tem * 0.5;PERLIN_NOISE(re_tem, perlin_poi * 8.0, r2);re = re + re_tem * 0.25;PERLIN_NOISE(re_tem, perlin_poi * 16.0, r3);re = re + re_tem * 0.125;PERLIN_NOISE(re_tem, perlin_poi * 32.0, r4);re = re + re_tem * 0.125;float4 sample_color;LINEAR_SAMPLE(sample_color, (poi_in + shift) / scale_in, texture_T, worley_size_I4);result_F = re * sample_color.x;}





















    //实际上也是长度在Local和在World下的比值
    float ray_local_length = length(ray_local);

    //计算射线穿过的距离，这里应该是World下的值（因为用的并不是单位化的Local Ray，而是变换过后的单位化World Ray，等于带上了缩放信息
    float ray_cross_length_world;
    {
        uint3 ray_step = step(ray_local, float3(0.0, 0.0, 0.0));
        float3 target_w = ray_step * min_width + (1 - ray_step) * max_width;
        //防除零，给加个极小值
        float3 eye_ray_normalize_local_3 = ((step(ray_local, 0.0) - 0.5) * -2.0) * max(abs(ray_local), 0.0000000001);
        float3 len = abs((target_w - ray_start_point_local) / ray_local);
        ray_cross_length_world = min(len.x, min(len.y, len.z));
    }

    //世界坐标系下的距离
    target_depth = min(ray_cross_length_world, target_depth);

    //return float4(target_depth / sqrt(12), 0.0, 0.0, 1.0);

    //局部坐标系下的距离
    float loacal_target_depth = target_depth * ray_local_length;

    //步进光线
    float3 ray = normalize(ray_local) * loacal_target_depth / 63.0;

    float ray_length = target_depth / 63.0;




    float Noise;
    CALCULATE_NOISE(Noise, ray_start_point_local, ten, scale, worley_size, float3(0.123, 0.456, 0.789), float3(0.123, 0.456, 0.789), float3(0.12233, 0.45456, 0.78967), float3(3.123, 2.456, 0.7823449), float3(1.0, 0.0, 0.0), time);

    /*
    //计算对应的worley坐标
    float3 Worley;
    COORDINATE_F3_TO_F3(Worley, (ray_start_point_local + 1.0 ) / 2.0, worley_size);





    //线性采样一下
    float4 Sample_Temporary;
    LINEAR_SAMPLE(Sample_Temporary, Worley, ten, worley_size);
*/

    float last_density = Noise * max_density;
    float decay = 0.0;
    float color = 0.0;

    float HG_V; 
    HG(HG_V, local_light, ray_local, 0.2);
    
    uint count = 0;
    for (count = 1; count < 63; ++count)
    {
        float3 poi = ray_start_point_local + ray * count;
        float Noise;
        CALCULATE_NOISE(Noise, poi, ten, scale, worley_size, float3(0.123, 0.456, 0.789), float3(0.123, 0.456, 0.789), float3(0.12233, 0.45456, 0.78967), float3(3.123, 2.456, 0.7823449), float3(1.0, 0.0, 0.0), time);

        float now_density = Noise * max_density;
        float current_decay = (now_density + last_density ) / 2.0 * ray_length;
        color = color + (1.0 - exp(-current_decay * 100.0)) * (exp(-decay)) * HG_V;
        decay = decay + current_decay;
        //decay = decay + now_density * ray_length;
        last_density = now_density;
    }

    float e_decay = exp(-decay);
    float inv_decay = 1.0 - e_decay;
    float inv_decay2 = inv_decay;
    float c = clamp((1.0 + exp(-decay)) * HG_V, 0.0, 1.0);
    return float4(c, c, c, inv_decay);
}