#pragma once
#include <ostream>
#include "define_dx.h"
namespace PO::Dx
{

	struct alignas(16) quaternions
	{
		// { {v -> }, s }
		float4a storage;
		vector operator*(const quaternions&) const noexcept;
		vector inverrse() const noexcept;
		float length() const noexcept;
		vector conjugate() const noexcept;
	};

	struct alignas(16) rotation_axis
	{
		//{{axis ->}, angle}
		float4a storage;
		rotation_axis(float3a axis, float angle) : storage{ axis.x, axis.y, axis.z, angle } {}
		rotation_axis(const rotation_axis&) = default;
		rotation_axis& operator=(const rotation_axis&) = default;
		vector to_quaternions() const noexcept;
	};

	struct alignas(16) transformation3D
	{
		float4x4a m_matrix;
		uint64_t m_vision = 0;
		transformation3D();
		transformation3D(float3a position, float3a rorate);
		matrix inverse() const;

		//void 
		/*
		transfer3D(float3 p, float3 s, const quaternions& e) : poi(p), sca(s), qua(e) {}
		transfer3D() = default;
		transfer3D(const transfer3D&) = default;
		transfer3D& set_eul(const eulerian_angle& e) { qua = e; return *this; }
		float4x4 inverse_float4x4() const;
		matrix inverse_matrix() const;
		operator float4x4() const { return qua.to_float4x4(sca, poi); }
		operator matrix() const { return qua.to_XMMATRIX(sca, poi); }
		*/
	};

	struct alignas(16) tranformation3D_sub : transformation3D
	{
		float4x4a m_final_matrix;
		uint64_t main_tran3d_vision = 0;
		uint64_t m_final_vision = 0;
		tranformation3D_sub();
		tranformation3D_sub(float3a position, float3a rorate) : transformation3D(position, rorate) {}
		matrix inverse() const;
		void update(const transformation3D& tf);
	};
}


/*
namespace PO
{
	namespace Dx
	{
		struct quaternions
		{
			// { {v -> }, s }
			vector storage;
			quaternions operator*(const quaternions&) const noexcept;
		};







		struct alignas(alignof(DirectX::XMVECTOR)) quaternions_template
		{
			// { {v -> }, s }
			DirectX::XMVECTOR store;
			quaternions_template operator* (const quaternions_template& qt) const;
			quaternions_template inverrse() const;
			float length() const;
			quaternions_template conjugate() const;
		};

		struct quaternions
		{
			// { {v -> }, s }
			float4a store;
			operator float4x4() const { return to_float4x4({ 1.0, 1.0, 1.0 }, { 0.0f, 0.0f, 0.0f }); }
			operator DirectX::XMMATRIX() const { return to_XMMATRIX({ 1.0, 1.0, 1.0 }, { 0.0f, 0.0f, 0.0f }); }
			float4x4 to_float4x4(float3 scale, float3 posi) const;
			DirectX::XMMATRIX to_XMMATRIX(float3 scale, float3 posi) const;
			quaternions(const quaternions_template& qt);
			quaternions& operator=(const quaternions_template& qt);
			quaternions& operator=(const quaternions&) = default;
			float3 rate(float3 v) const;
			operator quaternions_template() const;
			quaternions(const quaternions&) = default;
			quaternions(float r = 1.0f, float3 i = { 0.0, 0.0, 0.0 }) : store(i.x, i.y, i.z, r) {}
			quaternions_template operator* (const quaternions_template& q) const { return static_cast<quaternions_template>(*this) * q; }
		};

		struct eulerian_angle
		{
			float3 angle;
			eulerian_angle(float3 a = { 0.0f, 0.0f, 0.0f }) : angle(a) {}
			eulerian_angle(const eulerian_angle&) = default;
			eulerian_angle(const quaternions_template& q);
			operator quaternions_template() const;
			quaternions_template operator* (const quaternions_template& q) const { return static_cast<quaternions_template>(*this) * q; }
		};

		struct rotation_axis
		{
			//degree measure
			float angle;
			float3 axis;
			rotation_axis(float a, float3 ax) : angle(a), axis(ax) {}
			operator quaternions_template() const;
			quaternions_template operator* (const quaternions_template& q) const { return static_cast<quaternions_template>(*this) * q; }
		};

		struct transfer3D
		{
			float3 poi = { 0.0f, 0.0f, 0.0f };
			float3 sca = { 1.0f, 1.0f, 1.0f };
			quaternions qua = { 1.0f,{ 0.0f, 0.0f, 0.0f } };

			transfer3D(float3 p, float3 s, const quaternions& e) : poi(p), sca(s), qua(e) {}
			transfer3D() = default;
			transfer3D(const transfer3D&) = default;
			transfer3D& set_eul(const eulerian_angle& e) { qua = e; return *this; }
			float4x4 inverse_float4x4() const;
			matrix inverse_matrix() const;
			operator float4x4() const { return qua.to_float4x4(sca, poi); }
			operator matrix() const { return qua.to_XMMATRIX(sca, poi); }
		};

	}
}

std::ostream& operator<<(std::ostream& o, const PO::float2& m);
std::ostream& operator<<(std::ostream& o, const PO::float3& m);
std::ostream& operator<<(std::ostream& o, const PO::float4& m);
std::ostream& operator<<(std::ostream& o, const PO::float4x4& m);
std::ostream& operator<<(std::ostream& o, const PO::uint32_t3& m);
inline std::ostream& operator<<(std::ostream& o, const PO::Dx::eulerian_angle& e) { return o << e.angle; }
std::ostream& operator<<(std::ostream& o, const PO::Dx::quaternions& m);

inline PO::Dx::quaternions_template operator*(const PO::Dx::quaternions& q1, const PO::Dx::quaternions& q2)
{
	return PO::Dx::quaternions_template(q1) * PO::Dx::quaternions_template(q2);
}
*/

