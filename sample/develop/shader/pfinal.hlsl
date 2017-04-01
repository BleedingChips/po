struct in_ver
{
	float4 hPosition:SV_POSITION;
	float2 texCoords:TEXCOORD;
	float lowResTexCoords : LowRes;
	float4 clip_pos : CLIP;
	float2 depth_texc : DEPTH_TEXCOORD;
};

struct DepthIntepretationStruct
{
	float4 depthToLinFadeDistParams;
	bool reverseDepth;
};

struct TwoColourCompositeOutput
{
	float4 add		:SV_TARGET0;
	float4 multiply	:SV_TARGET1;
};

float4x4 worldToScatteringVolumeMatrix = 

float4x4(3.33244e-05, 0, -7.70392e-07, 0, 3.01428e-07, 3.06759e-05, 1.30387e-05, 0, 3.91368e-06, -7.19954e-05, 0.000169292, 0, -0, -0, -0, 1);

float4x4 invViewProj =

float4x4(-0.0254603, -0.0468672, 0, 0.996687, -0.999675, 0.0011937, 0, -0.0253842, 9.3534e-08, 0.605025, 0, 0.0772568, 0, 0, 0.999997, 3.33333e-06);

//float4x4(-0.016129, -0.99987, 1.27161e-09, 0, 0.0283048, -0.000456586, 0.606179, 0, 0, 0, 0, 0.999997, 0.998781, -0.0161114, -0.046649, 3.33333e-06);


float4x4 invShadowMatrix =



float4x4(2e-05, 0, 0, 0.5, 0, 2e-05, 0, 0.5, 0, 0, 0.0002, -0.6, 0, 0, 0, 1);
float4 viewportToTexRegionScaleBias = float4(1, 1, 0, 0);

float2 offset = float2(0, 0);
float alpha = 0;
float cloudShadowStrength = 0.5;

float4 colour2 = float4(0, 0, 0, 0);

float2 tanHalfFov = float2(1, 0.606839);
float exposure = 1;
float gamma = 1;

float4 depthToLinFadeDistParams = float4(1, 300000, 1, 0);
float4 warpHmdWarpParam = float4(0, 0, 0, 0);

float2 lowResTexelSizeX = float2(0, 0);
float2 warpLensCentre = float2(0, 0);

float2 warpScreenCentre = float2(0, 0);
float2 warpScale = float2(0, 0);

float2 warpScaleIn = float2(0, 0);
float2 padHdrConstants1 = float2(0, 0);

uint2 hiResDimsX = uint2(0, 0);
uint2 lowResDims = uint2(161, 121);

uint2 fullResDims = uint2(640, 480);
int numSamples = 0;
float maxFadeDistanceKm = 300;

float3 infraredIntegrationFactors = float3(0, 0, 0);
int randomSeed = 0;

float3 viewPos = float3(1.23327e-09, 4.84141e-08, 2000);
float nearDist = 0.000333333;

SamplerState cube_sample : register(s0);
SamplerState wmc_sampler : register(s1);
SamplerState wcc_sampler : register(s2);
SamplerState wrap_sampler : register(s3);
SamplerState cmc_sampler : register(s4);

#define texture_cube_lod(tex,texc,lod) tex.Sample(cube_sample,texc);
#define texture_3d_wmc_lod(tex,texc,lod) tex.Sample(wmc_sampler,texc)
#define texture_3d_wcc_lod(tex,texc,lod) tex.Sample(wcc_sampler,texc)
#define texture_wrap_lod(tex,texc,lod) tex.Sample(wrap_sampler,texc)
#define texture_clamp_mirror_lod(tex,texc,lod) tex.Sample(cmc_sampler,texc)
#define pi 3.141592653

texture2D depthTexture : register(t0);
TextureCube farImageTexture: register(t1);
TextureCube nearImageTexture: register(t2);
TextureCube nearFarTexture: register(t3);
TextureCube lightpassTexture: register(t4);
Texture2D loss2dTexture: register(t5);
Texture3D inscatterVolumeTexture: register(t6);
Texture3D godraysVolumeTexture: register(t7);
Texture2D shadowTexture: register(t8);


float depthToFadeDistance(float depth, float2 xy, DepthIntepretationStruct dis, float2 tanHalf)
{
	if (dis.reverseDepth)
	{
		if (depth <= 0)
			return 1.0f;
	}
	else
	{
		if (depth >= 1.0)
			return 1.0;
	}
	float linearFadeDistanceZ = dis.depthToLinFadeDistParams.x / (depth*dis.depthToLinFadeDistParams.y + dis.depthToLinFadeDistParams.z) + dis.depthToLinFadeDistParams.w*depth;
	float Tx = xy.x*tanHalf.x;
	float Ty = xy.y*tanHalf.y;
	float fadeDist = linearFadeDistanceZ * sqrt(1.0 + Tx*Tx + Ty*Ty);
	return fadeDist;
}

TwoColourCompositeOutput CompositeAtmospherics(float4 clip_pos
	, TextureCube farCloudTexture
	, TextureCube nearCloudTexture
	, TextureCube nearFarTexture
	, TextureCube lightpassTexture
	, Texture2D loss2dTexture
	, float dist
	, float4x4 invViewProj
	, float3 viewPos
	, float4x4 invShadowMatrix
	, DepthIntepretationStruct dis
	, Texture3D inscatterVolumeTexture
	, Texture3D godraysVolumeTexture
	, Texture2D cloudShadowTexture
	, float maxFadeDistanceKm
	, float cloud_shadow
	, float nearDist)
{
	TwoColourCompositeOutput res;
	float3 view = normalize(mul(invViewProj, clip_pos).xyz);
	float sine = view.z;
	float4 nearFarCloud = texture_cube_lod(nearFarTexture, view, 0);

	
	float dist_rt = pow(dist, 0.5);
	float4 cloud = texture_cube_lod(farCloudTexture, view, 0);
	float3 offsetMetres = view*dist*1000.0*maxFadeDistanceKm;
	float3 lightspaceOffset = (mul(worldToScatteringVolumeMatrix, float4(offsetMetres, 1.0)).xyz);
	float3 worldspaceVolumeTexCoords = float3(atan2(view.x, view.y) / (2.0*pi), 0.5*(1.0 + 2.0*asin(sine) / pi), sqrt(dist));

	// cut-off at the edges.
	float r = length(lightspaceOffset);
	float3 lightspaceVolumeTexCoords = float3(frac(atan2(lightspaceOffset.x, lightspaceOffset.y) / (2.0*pi))
		, 0.5 + 0.5*asin(lightspaceOffset.z / length(lightspaceOffset))*2.0 / pi
		, r);
	float4 insc = texture_3d_wmc_lod(inscatterVolumeTexture, worldspaceVolumeTexCoords, 0);
	float4 godrays = texture_3d_wcc_lod(godraysVolumeTexture, lightspaceVolumeTexCoords, 0);
	insc *= godrays;
	float2 loss_texc = float2(dist_rt, 0.5*(1.f - sine));
	float hiResInterp = 1.0 - pow(saturate((nearFarCloud.x - dist) / max(0.00001, nearFarCloud.x - nearFarCloud.y)), 1.0);
	// we're going to do TWO interpolations. One from zero to the near distance,
	// and one from the near to the far distance.
	float nearInterp = pow(saturate((dist) / 0.0033), 1.0);
	nearInterp = saturate((dist - nearDist) / max(0.00000001, 2.0*nearDist));
	//
	float4 lp = texture_cube_lod(lightpassTexture, view, 0);
	cloud.rgb += lp.rgb;

	float4 cloudNear = texture_cube_lod(nearCloudTexture, view, 0);

	cloud = lerp(cloudNear, cloud, hiResInterp);
	cloud = lerp(float4(0, 0, 0, 1.0), cloud, nearInterp);

	float3 worldPos = viewPos + offsetMetres;
	float illum = 1.0;

	float3 texc = mul(invShadowMatrix, float4(worldPos, 1.0)).xyz;
	float4 texel = texture_wrap_lod(cloudShadowTexture, texc.xy, 0);
	float above = saturate((texc.z - 0.5) / 0.5);
	texel.a += above;
	illum = saturate(texel.a);

	float shadow = lerp(1.0, illum, cloud_shadow);

	insc.rgb *= cloud.a;

	insc += cloud;
	res.multiply = texture_clamp_mirror_lod(loss2dTexture, loss_texc, 0)*cloud.a*shadow;
	res.add = insc;//vec4(lightspaceVolumeTexCoords,1.0);
	return res;
}

float4 yui(float4 u)
{
	return u;
}


TwoColourCompositeOutput main(in_ver IN)
{
	DepthIntepretationStruct depthInterpretationStruct;
	depthInterpretationStruct.depthToLinFadeDistParams = float4(1.0,300000, 1.0, 0.0);
	depthInterpretationStruct.reverseDepth = true;
	float depth = depthTexture.Sample(wrap_sampler, IN.depth_texc).x;
	float dist = depthToFadeDistance(depth, IN.clip_pos.xy, depthInterpretationStruct, tanHalfFov);
	TwoColourCompositeOutput result = CompositeAtmospherics(IN.clip_pos
		, farImageTexture
		, nearImageTexture
		, nearFarTexture
		, lightpassTexture
		, loss2dTexture
		, dist
		, invViewProj
		, viewPos
		, invShadowMatrix
		, depthInterpretationStruct
		, inscatterVolumeTexture
		, godraysVolumeTexture
		, shadowTexture
		, maxFadeDistanceKm
		, cloudShadowStrength


		, nearDist);
	result.add.rgb = pow(result.add.rgb, float3(gamma, gamma, gamma));
	result.add.rgb *= exposure;
	//return result;


	//float4 view = float4((normalize(mul(invViewProj, IN.clip_pos).xyz) + float3(1.0,1.0,1.0)) / 2.0, 0.0);
	//result.add = float4(nearFarTexture.Sample(cube_sample, view).xyz, 1.0);
	//result.add = float4(1.0, 1.0, 1.0, 1.0);
	//result.add = float4((IN.clip_pos + float2(1.0 , 1.0) ) / 2.0, 0.0, 1.0);
	//result.add = float4(1.0, 0.0, 0.0, 1.0);
	return result;

}