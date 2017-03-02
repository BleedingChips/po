#pragma once
[numthreads(8, 8, 1)]
void main(uint3 g : SV_GroupID, uint3 t : SV_GroupThreadID)
{
	uint3 dims;
	uint3 sub_pos = uint3(8 * g.x, g.y, g.z) + t;
	targetTexture.GetDimensions(dims.x, dims.y, dims.z);
	uint linear_pos = (sub_pos.x + threadOffset.x);
	int3 pos = LinearThreadToPos2D(int(linear_pos), int3(dims));
	if (pos.x >= targetSize.x || pos.y >= targetSize.y)
		return;
	IMAGE_STORE_3D(targetTexture, pos, vec4(1.0, 0.0, 0.0, 1.0));

	vec2 texc = (pos.xy + vec2(0.5, 0.5)) / vec2(targetSize.xy);
	vec4 previous_loss = vec4(1.0, 1.0, 1.0, 1.0);
	float sin_e = max(-1.0, min(1.0, 1.0 - 2.0*(texc.y*texSize.y - texelOffset) / (texSize.y - 1.0)));
	float cos_e = sqrt(1.0 - sin_e*sin_e);
	float altTexc = (texc.x*texSize.x - texelOffset) / max(texSize.x - 1.0, 1.0);
	float viewAltKm = texcToAltKm(altTexc, minOutputAltKm, maxOutputAltKm);
	float spaceDistKm = getDistanceToSpace(sin_e, viewAltKm);
	float prevDist_km = 0.0;
	for (uint i = 0; i<targetSize.z; i++)
	{
		uint3 idx = uint3(pos.xy, i);
		float zPosition = pow(float(i) / (float(targetSize.z) - 1.0), 2.0);
		float dist_km = zPosition*maxDistanceKm;
		if (i == targetSize.z - 1)
			dist_km = 12000.0;
		float maxd = min(spaceDistKm, dist_km);
		float mind = min(spaceDistKm, prevDist_km);
		float dist = 0.5*(mind + maxd);
		float stepLengthKm = max(0.0, maxd - mind);
		float y = planetRadiusKm + viewAltKm + dist*sin_e;
		float x = dist*cos_e;
		float r = sqrt(x*x + y*y);
		float alt_km = r - planetRadiusKm;
		// lookups is: dens_factor,ozone_factor,haze_factor;
		float dens_texc = saturate((alt_km / maxDensityAltKm*(tableSize.x - 1.0) + texelOffset) / tableSize.x);
		vec4 lookups = texture_clamp_lod(density_texture, vec2(dens_texc, .5), 0);
		float dens_factor = lookups.x;
		float ozone_factor = lookups.y;
		float haze_factor = getHazeFactorAtAltitude(alt_km);
		vec3 extinction = dens_factor*rayleigh + haze_factor*hazeMie + ozone*ozone_factor;
		vec4 loss;
		loss.rgb = exp(-extinction*stepLengthKm);
		loss.a = (loss.r + loss.g + loss.b) / 3.0;

		loss *= previous_loss;



		IMAGE_STORE_3D(targetTexture, idx, vec4(loss.rgb, 1.0));
		prevDist_km = dist_km;
		previous_loss = loss;
	}
}