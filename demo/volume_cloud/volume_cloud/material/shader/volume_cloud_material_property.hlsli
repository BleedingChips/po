#ifndef VOLUMECLOUD_MATERIAL_PROPERTY_INCLUDE_HLSLI
#define VOLUMECLOUD_MATERIAL_PROPERTY_INCLUDE_HLSLI
struct property_rendder_2d_for_3d
{
    float3 min_width;
    float3 max_width;
    float3 light;
    float density;
};

float RayPatch(float3 RayWidthLengthInfo_F3, float3 StartPoint_F3, float3 WidthHeightDepth_F3)
{
    float3 RayStep = step(float3(0.0, 0.0, 0.0), RayWidthLengthInfo_F3);
    float3 RayEndSurface = lerp(-WidthHeightDepth_F3, WidthHeightDepth_F3, RayStep);
        //防除0
    float3 LocalEyeRayWithLengthInformationNoZero = ((step(0.0, RayWidthLengthInfo_F3) - 0.5) * 2.0) * max(abs(RayWidthLengthInfo_F3), 0.00001);
    float3 len = (RayEndSurface - StartPoint_F3) / LocalEyeRayWithLengthInformationNoZero;
    return max(min(len.x, min(len.y, len.z)), 0.0);
}

float3 PreventZero(float3 Input_F3)
{
    return lerp(0.00001, Input_F3, step(0.00001, Input_F3));
}


// for UE4 use
float4 Texture2DSample(Texture2D Tex, SamplerState Sampler, float2 UV)
{
    return Tex.Sample(Sampler, UV);
}

float UE4DefineFunction()
{
    return float(0.0);
}

float Sample2D4ChannelSimulate3D1Channel(Texture2D Tex, SamplerState SS, float3 SampleLocaltion, uint4 Block)
{
    float3 MirroLocation = abs(1.0 - fmod(abs(SampleLocaltion + 1.0), 2.0));

    float3 IntLoacation = MirroLocation * uint3(Block.x, Block.y, Block.z * Block.w * 4);

    uint ZChannelCount = Block.z * Block.w;
    uint ZCount = ZChannelCount * 4 - 1;
    float ZChunk = MirroLocation.z * ZCount;
    uint ZChunkLast = floor(ZChunk);
    uint ZChunkMax = ZChunkLast + 1;
    float rate = ZChunk - ZChunkLast;

    ZChunkMax = step(ZChunkMax, ZCount) * (ZChunkMax);
    uint2 LastChunkXY = uint2(ZChunkLast % Block.z, (ZChunkLast % ZChannelCount) / Block.z);
    uint2 MaxChunkXY = uint2(ZChunkMax % Block.z, (ZChunkMax % ZChannelCount) / Block.z);
    float2 FinalLocationLast = float2(
    (MirroLocation.x * 0.98 + 0.01 + LastChunkXY.x) / float(Block.z),
    (MirroLocation.y * 0.98 + 0.01 + LastChunkXY.y) / float(Block.w)
    );
    float2 FinalLocationMax = float2(
    (MirroLocation.x * 0.98 + 0.01 + MaxChunkXY.x) / float(Block.z),
    (MirroLocation.y * 0.98 + 0.01 + MaxChunkXY.y) / float(Block.w)
    );

    static const float4 IndexFactor[4] =
    {
        float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0), float4(0, 0, 0, 1)
    };

    float4 S1 = Texture2DSample(Tex, SS, FinalLocationLast);
    float4 S2 = Texture2DSample(Tex, SS, FinalLocationMax);
    float4 F1 = IndexFactor[ZChunkLast / ZChannelCount];
    float4 F2 = IndexFactor[ZChunkMax / ZChannelCount];

    float V1 = dot(S1, F1);
    float V2 = dot(S2, F2);

    return lerp(V1, V2, rate);
}


float RayMatchingWidthNoLighting(

// 基本形状属性
Texture2D BaseShape_T,
SamplerState BaseShape_TSampler,
float3 BaseShapeScale_F3,
float3 BaseShapeTransform_F3,
float2 BaseShapeValueFactor_F2,
float4 BaseShapeSimulateSize_F4,

// 移动的挖空属性
Texture2D MoveableMask_T,
SamplerState MoveableMask_TSampler,
float3 MoveableMaskSacle_F3,
float3 MoveableMaskTransform_F3,
float2 MoveableMaskValueFactor_F2,
float4 MoveableMaskSimulateSize_F4,

// 静止的挖空属性
Texture2D Mask_T,
SamplerState Mask_TSampler,
float2 MaskValueFactor_F2,
float4 MaskSimulateSize_F4,

float3 UnitPoint_F3,
float3 UnitRayPath_F3,
float Density_F,
float Time_F,
float3 UnitMove_F3
)
{
    uint4 MaskSimulateSize_U4 = MaskSimulateSize_F4;
    uint4 MoveableMaskSimulateSize_U4 = MoveableMaskSimulateSize_F4;
    uint4 BaseShapeSimulateSize_U4 = BaseShapeSimulateSize_F4;

    const uint SampleCount = 32;
    float RayPathLength = length(UnitRayPath_F3);
    float3 UnitRayStep = UnitRayPath_F3 / SampleCount;

    float ResultDensity = 0.0;
    float LastDensity = 0.0;

    float3 SamplePoint = UnitPoint_F3;
    uint count = 0;
    for (count = 0; count < (SampleCount - 1); ++count)
    {

        float3 ShiftSamplePoint = SamplePoint * BaseShapeScale_F3 + BaseShapeTransform_F3 + Time_F * UnitMove_F3;
        float BaseShape = dot(float2(Sample2D4ChannelSimulate3D1Channel(BaseShape_T, BaseShape_TSampler, ShiftSamplePoint, BaseShapeSimulateSize_U4), 1.0), BaseShapeValueFactor_F2);

        ShiftSamplePoint = SamplePoint * MoveableMaskSacle_F3 + MoveableMaskTransform_F3 + Time_F * UnitMove_F3;
        float MoveMask = dot(float2(Sample2D4ChannelSimulate3D1Channel(MoveableMask_T, MoveableMask_TSampler, ShiftSamplePoint, MoveableMaskSimulateSize_F4), 1.0), MoveableMaskValueFactor_F2);

        float Mask = dot(float2(Sample2D4ChannelSimulate3D1Channel(Mask_T, Mask_TSampler, SamplePoint, MaskSimulateSize_F4), 1.0), MaskValueFactor_F2);

        float FinalDensity = max(BaseShape - MoveMask, 0.0) * clamp(Mask, 0.0, 1.0);
        ResultDensity = ResultDensity + FinalDensity;
        (FinalDensity + LastDensity) / 2.0;
        LastDensity = FinalDensity;
        SamplePoint = SamplePoint + UnitRayStep;
    }

    float DensityE = exp(-ResultDensity * Density_F * RayPathLength);
    return 1.0 - DensityE;
}

//必须是从后面采样到前面
float2 RayMatchingWidth(

// 基本形状属性
Texture2D BaseShape_T,
SamplerState BaseShape_TSampler,
float3 BaseShapeScale_F3,
float3 BaseShapeTransform_F3,
float2 BaseShapeValueFactor_F2,
float4 BaseShapeSimulateSize_F4,

// 移动的挖空属性
Texture2D MoveableMask_T,
SamplerState MoveableMask_TSampler,
float3 MoveableMaskSacle_F3,
float3 MoveableMaskTransform_F3,
float2 MoveableMaskValueFactor_F2,
float4 MoveableMaskSimulateSize_F4,

// 静止的挖空属性
Texture2D Mask_T,
SamplerState Mask_TSampler,
float2 MaskValueFactor_F2,
float4 MaskSimulateSize_F4,

float3 UnitPoint_F3,
float3 UnitRayPath_F3,
float Density_F,
float Time_F,
float3 UnitMove_F3,
float3 UnitLight_F3
)
{
    uint4 MaskSimulateSize_U4 = MaskSimulateSize_F4;
    uint4 MoveableMaskSimulateSize_U4 = MoveableMaskSimulateSize_F4;
    uint4 BaseShapeSimulateSize_U4 = BaseShapeSimulateSize_F4;

    const uint SampleCount = 32;
    float RayPathLength = length(UnitRayPath_F3);
    float3 UnitRayStep = UnitRayPath_F3 / SampleCount;

    float ResultDensity = 0.0;
    float LastDensity = 0.0;

    float LightReflex = 0.0;

    float3 SamplePoint = UnitPoint_F3;
    uint count = 0;
    for (count = 0; count < (SampleCount - 1); ++count)
    {

        float3 ShiftSamplePoint = SamplePoint * BaseShapeScale_F3 + BaseShapeTransform_F3 + Time_F * UnitMove_F3;
        float BaseShape = dot(float2(Sample2D4ChannelSimulate3D1Channel(BaseShape_T, BaseShape_TSampler, ShiftSamplePoint, BaseShapeSimulateSize_U4), 1.0), BaseShapeValueFactor_F2);

        ShiftSamplePoint = SamplePoint * MoveableMaskSacle_F3 + MoveableMaskTransform_F3 + Time_F * UnitMove_F3;
        float MoveMask = dot(float2(Sample2D4ChannelSimulate3D1Channel(MoveableMask_T, MoveableMask_TSampler, ShiftSamplePoint, MoveableMaskSimulateSize_F4), 1.0), MoveableMaskValueFactor_F2);

        float Mask = dot(float2(Sample2D4ChannelSimulate3D1Channel(Mask_T, Mask_TSampler, SamplePoint, MaskSimulateSize_F4), 1.0), MaskValueFactor_F2);

        float FinalDensity = max(BaseShape - MoveMask, 0.0) * clamp(Mask, 0.0, 1.0);
        ResultDensity = ResultDensity + FinalDensity;
        (FinalDensity + LastDensity) / 2.0;
        LastDensity = FinalDensity;
        SamplePoint = SamplePoint + UnitRayStep;
    }

    float DensityE = exp(-ResultDensity * Density_F * RayPathLength);
    return 1.0 - DensityE;
}

void CalculateFlipNormalCubeRayStartEndLocalPosition(
out float3 StartLocalPosition, out float3 EndLocalPosition,
in float3 WidthHeightDepth,
in float ScreenDepth,
in float3 PixelWorldPosition,
in float3 CameraWorldPosition,
in float3 CameraWorldDir,
in float PixelDepth,
in float4x4 WorldToLocal,
in float NearClipPlane
)

{
    // 计算视角射线向量
    float3 EyeRay = PixelWorldPosition - CameraWorldPosition;

    // 计算像素点的深度与不透明物体的深度的最小值，计算射线的开始点。
    float PixelMinDepth = min(ScreenDepth, PixelDepth);

    // 通过向量和深度比计算实际采样射线的。
    float3 EndWorldPosition = EyeRay * (PixelMinDepth / PixelDepth) + CameraWorldPosition;

    EyeRay = normalize(EyeRay);

    // 世界坐标系下的路径与深度差的比值
    float Result = dot(EyeRay, normalize(CameraWorldDir));

    // 计算带长度信息的局部坐标下的光线向量
    float3 LocalEyeRayWithLengthInformation = mul(WorldToLocal, float4(EyeRay, 0.0)).xyz;

    // 计算结束点的局部坐标
    float4 EndWorldPosition4 = float4(EndWorldPosition, 1.0);
    EndWorldPosition4 = mul(WorldToLocal, EndWorldPosition4);
    EndLocalPosition = EndWorldPosition4.xyz / EndWorldPosition4.w;

    // 计算反向光线的位移，实际上就是从起始点，通过反向光线移动到立方体边界的位移
    float ReverseRayLenght = RayPatch(-LocalEyeRayWithLengthInformation, EndLocalPosition, WidthHeightDepth);

    // 减去近采样面的距离，然后计算在当前射线下的实际距离
    float MinWorldDepth = (PixelMinDepth - NearClipPlane) / Result;

    // 在世界坐标系下的最小深度
    float FinalWorldDepth = min(ReverseRayLenght, MinWorldDepth);

    StartLocalPosition = FinalWorldDepth * -LocalEyeRayWithLengthInformation + EndLocalPosition;
}


#endif