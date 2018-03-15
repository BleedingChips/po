#pragma once
#include <stdint.h>
#include <assert.h>
namespace PO
{
	/*
	using uint = uint32_t;

	struct uint2
	{
		uint32_t x, y;
		uint2(uint32_t i) : x(i), y(i) {}
		uint2() : x(0), y(0) {}
		uint2(uint32_t ix, uint32_t iy) noexcept : x(ix), y(iy) {}
		uint2(const uint2&) noexcept = default;
		uint2& operator= (const uint2&) noexcept = default;
		uint2 operator+(const uint2& o) const noexcept { return uint2{ x + o.x, y + o.y }; }
		uint2 operator-(const uint2& o) const noexcept { return uint2{ x - o.x, y - o.y }; }
	};

	struct uint3
	{
		uint32_t x, y, z;
		bool operator==(const uint3& o) const noexcept { return x == o.x && y == o.y && z == o.z; }
		uint3(uint32_t i) : x(i), y(i), z(i) {}
		uint3() : x(0), y(0), z(0) {}
		uint3(uint32_t ix, uint32_t iy, uint32_t iz) noexcept : x(ix), y(iy), z(iz) {}
		uint3(const uint3&) noexcept = default;
		uint3& operator= (const uint3&) noexcept = default;
		uint3 operator+(const uint3& o) const noexcept { return uint3{ x + o.x, y + o.y, z + o.z }; }
		uint3 operator-(const uint3& o) const noexcept { return uint3{ x - o.x, y - o.y, z - o.z }; }
		uint3 operator*(const uint3& o) const noexcept { return uint3{ x * o.x, y * o.y, z * o.z }; }
	};

	struct float2
	{
		float x, y;
		float2(float i) : x(i), y(i) {}
		float2() : x(0.0f), y(0.0f) {}
		float2(float ix, float iy) noexcept : x(ix), y(iy) {}
		float2(const float2&) noexcept = default;
		float2& operator= (const float2&) noexcept = default;
		float2 operator+(const float2& o) const noexcept { return float2{ x + o.x, y + o.y }; }
		float2 operator-(const float2& o) const noexcept { return float2{ x - o.x, y - o.y }; }
	};

	struct float3
	{
		float x, y, z;
		float3(float i) : x(i), y(i), z(i) {}
		float3() : x(0.0f), y(0.0f), z(0.0f) {}
		float3(float ix, float iy, float iz) noexcept : x(ix), y(iy), z(iz) {}
		float3(const float3&) noexcept = default;
		float3& operator= (const float3&) noexcept = default;
		float3 operator+(const float3& o) const noexcept { return float3{ x + o.x, y + o.y, z + o.z }; }
		float3 operator-(const float3& o) const noexcept { return float3{ x - o.x, y - o.y, z - o.z }; }
		float3 operator*(const float3& o) const noexcept { return float3{ x * o.x, y * o.y, z * o.z }; }
	};

	struct float4
	{
		float x, y, z, w;
		float4(float i) : x(i), y(i), z(i), w(i) {}
		float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
		float4(float ix, float iy, float iz, float iw) noexcept : x(ix), y(iy), z(iz), w(iw) {}
		float4(const float4&) noexcept = default;
		float4& operator= (const float4&) noexcept = default;
		float4 operator+(const float4& o) const noexcept { return float4{ x + o.x, y + o.y, z + o.z, w + o.w }; }
		float4 operator-(const float4& o) const noexcept { return float4{ x - o.x, y - o.y, z - o.z, w - o.w }; }
		float4 operator*(const float4& o) const noexcept { return float4{ x * o.x, y * o.y, z * o.z, w * o.w }; }
	};

	struct float4x4
	{
		float
			_11, _12, _13, _14,
			_21, _22, _23, _24,
			_31, _32, _33, _34,
			_41, _42, _43, _44;


		float4x4() noexcept :
			_11(1.0f), _12(0.0f), _13(0.0f), _14(0.0f),
			_21(1.0f), _22(1.0f), _23(0.0f), _24(0.0f),
			_31(1.0f), _32(0.0f), _33(1.0f), _34(0.0f),
			_41(1.0f), _42(0.0f), _43(0.0f), _44(1.0f)
		{}

		float4x4(
			float i_11, float i_12, float i_13, float i_14,
			float i_21, float i_22, float i_23, float i_24,
			float i_31, float i_32, float i_33, float i_34,
			float i_41, float i_42, float i_43, float i_44
		) :
			_11(i_11), _12(i_12), _13(i_13), _14(i_14),
			_21(i_21), _22(i_22), _23(i_23), _24(i_24),
			_31(i_31), _32(i_32), _33(i_33), _34(i_34),
			_41(i_41), _42(i_42), _43(i_43), _44(i_44)
		{}
		float4x4(const float4x4&) noexcept = default;
		float& operator()(size_t x, size_t y) noexcept {
			assert(x < 4 && y < 4);
			return (&_11)[x + y * 4];
		}
		const float& operator()(size_t x, size_t y) const noexcept {
			assert(x < 4 && y < 4);
			return (&_11)[x + y * 4];
		}
	};

	struct quaternions
	{
		// { {v -> }, s }
		float4 store;
		operator float4x4() const { return to_float4x4({ 1.0, 1.0, 1.0 }, { 0.0f, 0.0f, 0.0f }); }
		float4x4 to_float4x4(float3 scale, float3 posi) const;
		quaternions& operator=(const quaternions&) = default;
		float3 rate(float3 v) const;
		quaternions(const quaternions&) = default;
		quaternions(float r = 1.0f, float3 i = { 0.0, 0.0, 0.0 }) : store(i.x, i.y, i.z, r) {}
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
	*/

}