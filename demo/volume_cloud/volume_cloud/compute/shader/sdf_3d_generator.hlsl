Texture3D inputTexture : register(t0);
RWTexture3D<float4> InisideTexture : register(u0);
RWTexture3D<float4> OutsideTexture : register(u1);

cbuffer b0 : register(b0)
{
    uint3 inputblock_start;
    uint3 inputblock_end;
    uint3 inputsize;
    uint3 outputsize;
    float EdgeValue;
    float3 DistanceMulity;
    uint final;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (dot(step(DTid, outputsize - 1), 1) == 3)
    {
        if(final == 0)
        {
            float3 Poi = uint3(DTid) / float3(outputsize - 1);

    //(Euclidean, Manhattan, Chessboard)

            float3 BigerValue = InisideTexture[DTid].xyz;
            float3 SmallValue = OutsideTexture[DTid].xyz;

            for (uint countX = inputblock_start.x; countX < inputblock_end.x; ++countX)
            {
                for (uint countY = inputblock_start.y; countY < inputblock_end.y; ++countY)
                {
                    for (uint countZ = inputblock_start.z; countZ < inputblock_end.z; ++countZ)
                    {
                        uint3 index_point = uint3(countX, countY, countZ);
                        float Value = inputTexture[index_point].x;
                        float3 CurPoi = float3(countX, countY, countZ) / float3(inputsize - 1);
                        float3 CurDir = abs(Poi - CurPoi);
                        CurDir = min(CurDir, 1.0 - CurDir);
                        float3 ThreeKindOfDistance = float3(length(CurDir), CurDir.x + CurDir.y + CurDir.z, max(CurDir.x, max(CurDir.y, CurDir.z)));
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
            }

            InisideTexture[DTid] = float4(BigerValue, 1.0);
            OutsideTexture[DTid] = float4(SmallValue, 1.0);

        }
        else
        {
            float3 BigerValue = InisideTexture[DTid].xyz;
            float3 SmallerValue = OutsideTexture[DTid].xyz;
            float3 FinalValue = max(BigerValue, SmallerValue) * (step(BigerValue, SmallerValue) * 2 - 1.0);
            FinalValue = (FinalValue * DistanceMulity + 1.0) / 2.0;
            InisideTexture[DTid] = float4(FinalValue, 1.0);
        }
        
        //InisideTexture[DTid] = float4(BigerValue * DistanceMulity, 1.0);
       //OutsideTexture[DTid] = float4(SmallValue * DistanceMulity, 1.0);

        /*
        float3 FinalValue = max(BigerValue, SmallValue) * (step(SmallValue, BigerValue) * 2 - 1.0);
        float3 OldValue = (outputTexture[DTid].xyz / float(255) - 0.5) * 2.0 / DistanceMulity;
        FinalValue = lerp(FinalValue, OldValue, step(abs(OldValue), abs(FinalValue)));
        FinalValue = clamp((FinalValue * DistanceMulity + 1.0) / 2.0, 0.0, 1.0);
        outputTexture[DTid] = float4(FinalValue * 255, 255);
*/
    }
    
}