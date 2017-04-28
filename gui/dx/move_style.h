#pragma once
#include "dx_math.h"
#include <chrono>
namespace PO
{
	namespace Dx
	{

		struct opposite_direct
		{
			//positive 
			int8_t direction = 0; // 00XY X is positive, Y is negetive 
			void set_positive() { direction = direction | 0x2; }
			void unset_positive() { direction = direction & 0x1; }
			void set_negetive() { direction = direction | 0x1; }
			void unset_negetive() { direction = direction & 0x2; }
			void positive(bool s) { s ? set_positive() : unset_positive(); }
			void negetive(bool s) { s ? set_negetive() : unset_negetive(); }
			int final_direction() const;
		};

		struct alignas(alignof(vector)) movement_interpolation
		{

			float3 poi = {0.0f, 0.0f, 0.0f};
			float3 sca = {1.0f, 1.0f, 1.0f};
			float3 eul = {0.0f, 0.0f, 0.0f};

			movement_interpolation(float3 p, float3 s, float3 e) : poi(p), sca(s), eul(e) {}
			movement_interpolation() = default;
			movement_interpolation(const movement_interpolation&) = default;
			void set_sca(float3 c) { sca = c; }
			void set_eul(float3 e) { eul = e; }
			operator float4x4() const;
			operator matrix() const;

		};
	}
}