/*
float4 Last_Density_Decay_Factor_And_Len;
float2 ThisFactor;
*/

float4 LDDFAL = Last_Density_Decay_Factor_And_Len;

float F = LDDFAL.x - log((LDDFAL.z + ThisFactor.x) / 2.0) * LDDFAL.w;

return float4(
    F,
    1.0, //LDDFAL.y + exp(-F) * (1.0 - exp(log(LDDFAL.z) * LDDFAL.w)) * ThisFactor.y * 1.2, 
    ThisFactor.x,
    LDDFAL.w
);
