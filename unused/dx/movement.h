#pragma once
#include "transformation.h"
#include <atomic>
#include <chrono>
namespace PO
{
	namespace Dx
	{
		/*
		struct opposite_direct
		{
			//positive 
			std::atomic<int8_t> direction = 0; // 00XY X is positive, Y is negetive 
			void set_positive() { direction = direction | 0x2; }
			void unset_positive() { direction = direction & 0x1; }
			void set_negetive() { direction = direction | 0x1; }
			void unset_negetive() { direction = direction & 0x2; }
			void positive(bool s) { s ? set_positive() : unset_positive(); }
			void negetive(bool s) { s ? set_negetive() : unset_negetive(); }
			int final_direction() const;
		};

		struct movement_free_object
		{
			float3 poi = { 0.0f, 0.0f, 0.0f };
			float3 sca = { 1.0, 1.0, 1.0 };
			quaternions qua = {};
			operator float4x4() const { return qua.to_float4x4(sca, poi); }
			operator matrix() const { return qua.to_XMMATRIX(sca, poi); }
		};

		struct alignas(alignof(vector)) movement_interpolation
		{

			float3 poi = {0.0f, 0.0f, 0.0f};
			float3 sca = {1.0f, 1.0f, 1.0f};
			quaternions qua = { 0.0f, {1.0f, 0.0f, 0.0f} };

			movement_interpolation(float3 p, float3 s, const quaternions& e) : poi(p), sca(s), qua(e) {}
			movement_interpolation() = default;
			movement_interpolation(const movement_interpolation&) = default;
			movement_interpolation& set_eul(const eulerian_angle& e) { qua = e; return *this; }
			operator float4x4() const { return qua.to_float4x4(sca, poi); }
			operator matrix() const { return qua.to_XMMATRIX(sca, poi); }

		};
		*/
	}
}