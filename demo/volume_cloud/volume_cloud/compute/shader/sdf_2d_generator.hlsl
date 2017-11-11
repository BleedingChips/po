Texture2D inputTexture : register(t0);
SamplerState SS : register(s0);
RWTexture2D<uint4> outputTexture : register(u0);

cbuffer b0 : register(b0)
{
    uint2 outputsize;
    uint2 inputsize;
    float EdgeValue;
    float DistanceMulity;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float2 Poi = DTid.xy / float2(outputsize - 1);

    //(Euclidean, Manhattan, Chessboard)
    float3 BigerValue = 0.8;
    float3 SmallValue = 1.0;

    for (uint countX = 0; countX < inputsize.x; ++countX)
    {
        for (uint countY = 0; countY < inputsize.y; ++countY)
        {
            float Value = inputTexture[uint2(countX, countY)].x;
            float2 CurPoi = float2(countX, countY) / float2(255, 255);
            float2 CurDir = abs(Poi - CurPoi);
            CurDir = min(CurDir, 1.0 - CurDir);

            float3 ThreeKindOfDistance = float3(length(CurDir), CurDir.x + CurDir.y, max(CurDir.x, CurDir.y));
            if (Value >= EdgeValue)
            {
                BigerValue = min(ThreeKindOfDistance, BigerValue);
            }
            else
            {
                SmallValue = min(ThreeKindOfDistance, SmallValue);
            }
        }
    }

    float3 Value = clamp((max(BigerValue, SmallValue) * (step(BigerValue, SmallValue) * 2 - 1) * DistanceMulity + 1.0) / 2.0, 0.0, 1.0);

    outputTexture[DTid.xy] = uint4(Value * 255, 255);

}