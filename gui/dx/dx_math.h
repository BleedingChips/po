#pragma once
#include <ostream>
#include "dx_type.h"
namespace PO
{
	namespace Dx
	{

		struct alignas(alignof(DirectX::XMVECTOR)) quaternions_template
		{
			// { {v -> }, s }
			DirectX::XMVECTOR store;
			quaternions_template operator* (const quaternions_template& qt) const;
		};

		struct quaternions
		{
			// { {v -> }, s }
			float4 store;
			//quaternions operator* (const quaternions& m);
			//float3 tarnfer(const float3& m);
			operator float4x4() const { return to_float4x4({ 1.0, 1.0, 1.0 }, { 0.0f, 0.0f, 0.0f }); }
			operator DirectX::XMMATRIX() const { return to_XMMATRIX({ 1.0, 1.0, 1.0 }, { 0.0f, 0.0f, 0.0f }); }
			float4x4 to_float4x4(float3 scale, float3 posi) const;
			DirectX::XMMATRIX to_XMMATRIX(float3 scale, float3 posi) const;
			quaternions(const quaternions_template& qt);
			operator quaternions_template() const;
			quaternions(const quaternions&) = default;
			quaternions(float r = 1.0f, float3 i = {0.0, 0.0, 0.0}) : store(i.x, i.y, i.z, r) {}
			quaternions_template operator* (const quaternions_template& q) const { return static_cast<quaternions_template>(*this) * q; }
		};

		struct eulerian_angle
		{
			float3 angle;
			operator float4x4() const;
			operator DirectX::XMMATRIX() const;
			//operator quaternions() const;
			eulerian_angle(float3 a = { 0.0f, 0.0f, 0.0f }) : angle(a) {}
			eulerian_angle(const eulerian_angle&) = default;
			eulerian_angle(const quaternions_template& q);
			operator quaternions_template() const;
			quaternions_template operator* (const quaternions_template& q) const { return static_cast<quaternions_template>(*this) * q; }
		};

		struct rotation_axis
		{
			float angle;
			float3 axis;
			rotation_axis(float a, float3 ax) : angle(a), axis(ax) {}
			operator quaternions_template() const;
			quaternions_template operator* (const quaternions_template& q) const { return static_cast<quaternions_template>(*this) * q; }
		};

	}
}

std::ostream& operator<<(std::ostream& o, const PO::Dx::float2& m);
std::ostream& operator<<(std::ostream& o, const PO::Dx::float3& m);
std::ostream& operator<<(std::ostream& o, const PO::Dx::float4& m);
std::ostream& operator<<(std::ostream& o, const PO::Dx::float4x4& m);
inline std::ostream& operator<<(std::ostream& o, const PO::Dx::eulerian_angle& e) { return o << e.angle; }
std::ostream& operator<<(std::ostream& o, const PO::Dx::quaternions& m);

inline PO::Dx::quaternions_template operator*(const PO::Dx::quaternions& q1, const PO::Dx::quaternions& q2)
{
	return PO::Dx::quaternions_template(q1) * PO::Dx::quaternions_template(q2);
}

