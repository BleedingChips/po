#pragma once

namespace PO ::Graphic
{

	







	/*
	struct float2
	{
		float x, y;
		float2(float input = 0.0f) : x(input), y(input) {}
		float2(float ix, float iy) : x(ix), y(iy) {}
		float2(const float2&) = default;

		float2 operator-(float2 i) { return float2{ x - i.x, y - i.y }; }
		float2 operator+(float2 i) { return float2{ x + i.x, y + i.y }; }
		float2 operator*(float2 i) { return float2{ x * i.x, y * i.y }; }
		float2 operator/(float2 i) { return float2{ x / i.x, y / i.y }; }
		float2& operator= (const float2&) = default;
	};

	struct float3
	{
		float x, y, z;
		float3(float i = 0.0f) : x(i), y(i), z(i) {}
		float3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {}
		float3(float2 ixy, float iz) : x(ixy.x), y(ixy.y), z(iz) {}
		float3(float i, float2 iyz) : x(i), y(iyz.x), z(iyz.y) {}
		float3(const float3&) = default;
		float3& operator= (const float3&) = default;

		float3 operator-(float3 i) { return float3{ x - i.x, y - i.y, z - i.z }; }
		float3 operator+(float3 i) { return float3{ x + i.x, y + i.y, z + i.z }; }
		float3 operator*(float3 i) { return float3{ x * i.x, y * i.y, z * i.z }; }
		float3 operator/(float3 i) { return float3{ x / i.x, y / i.y, z / i.z }; }
	};

	struct float4
	{
		float x, y, z, w;
		float4(float i = 0.0f) : x(i), y(i), z(i), w(i) {}
		float4(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) {}
		float4(float2 ixy, float iz, float iw) : x(ixy.x), y(ixy.y), z(iz), w(iw) {}
		float4(float i, float2 iyz, float iw) : x(i), y(iyz.x), z(iyz.y), w(iw) {}
		float4(float ix, float iy, float2 izw) : x(ix), y(iy), z(izw.x), w(izw.y) {}
		float4(float2 ixy, float2 izw) : x(ixy.x), y(ixy.y), z(izw.x), w(izw.y) {}
		float4(float3 ixyz, float iw) : x(ixyz.x), y(ixyz.y), z(ixyz.z), w(iw) {}
		float4(float ix, float3 iyzw) : x(ix), y(iyzw.x), z(iyzw.y), w(iyzw.z) {}
		float4(const float4&) = default;
		float4& operator= (const float4&) = default;

		float4 operator-(float4 i) { return float4{ x - i.x, y - i.y, z - i.z, w - i.w }; }
		float4 operator+(float4 i) { return float4{ x + i.x, y + i.y, z + i.z, w + i.w }; }
		float4 operator*(float4 i) { return float4{ x * i.x, y * i.y, z * i.z, w * i.w }; }
		float4 operator/(float4 i) { return float4{ x / i.x, y / i.y, z / i.z, w / i.w }; }
	};

	using uint = uint32_t;

	struct uint2
	{
		uint x, y;
		uint2(uint input = 0.0f) : x(input), y(input) {}
		uint2(uint ix, uint iy) : x(ix), y(iy) {}
		uint2(const uint2&) = default;
		uint2& operator= (const uint2&) = default;
		uint2(float2 i) : x(static_cast<uint>(i.x)), y(static_cast<uint>(i.y)) {}

		operator float2() const { return float2{ x, y }; }
		uint2 operator-(uint2 i) { return uint2{ x - i.x, y - i.y }; }
		uint2 operator+(uint2 i) { return uint2{ x + i.x, y + i.y }; }
		uint2 operator*(uint2 i) { return uint2{ x * i.x, y * i.y }; }
		uint2 operator/(uint2 i) { return uint2{ x / i.x, y / i.y }; }
		bool operator==(uint2 i) { return x == i.x && x == i.y; }

	};

	struct uint3
	{
		uint x, y, z;
		uint3(uint i = 0.0f) : x(i), y(i), z(i) {}
		uint3(uint ix, uint iy, uint iz) : x(ix), y(iy), z(iz) {}
		uint3(uint2 ixy, uint iz) : x(ixy.x), y(ixy.y), z(iz) {}
		uint3(uint i, uint2 iyz) : x(i), y(iyz.x), z(iyz.y) {}
		uint3(const uint3&) = default;
		uint3(float3 i) : x(static_cast<uint>(i.x)), y(static_cast<uint>(i.y)), z(static_cast<uint>(i.z)) {}
		uint3& operator= (const uint3&) = default;

		operator float3() const { return float3{ x, y, z }; }
		uint3 operator-(uint3 i) { return uint3{ x - i.x, y - i.y, z - i.z }; }
		uint3 operator+(uint3 i) { return uint3{ x + i.x, y + i.y, z + i.z }; }
		uint3 operator*(uint3 i) { return uint3{ x * i.x, y * i.y, z * i.z }; }
		uint3 operator/(uint3 i) { return uint3{ x / i.x, y / i.y, z / i.z }; }
		bool operator==(uint3 i) { return x == i.x && x == i.y && z == i.z; }
	};

	struct uint4
	{
		uint x, y, z, w;
		uint4(uint i = 0.0f) : x(i), y(i), z(i), w(i) {}
		uint4(uint ix, uint iy, uint iz, uint iw) : x(ix), y(iy), z(iz), w(iw) {}
		uint4(uint2 ixy, uint iz, uint iw) : x(ixy.x), y(ixy.y), z(iz), w(iw) {}
		uint4(uint i, uint2 iyz, uint iw) : x(i), y(iyz.x), z(iyz.y), w(iw) {}
		uint4(uint ix, uint iy, uint2 izw) : x(ix), y(iy), z(izw.x), w(izw.y) {}
		uint4(uint2 ixy, uint2 izw) : x(ixy.x), y(ixy.y), z(izw.x), w(izw.y) {}
		uint4(uint3 ixyz, uint iw) : x(ixyz.x), y(ixyz.y), z(ixyz.z), w(iw) {}
		uint4(uint ix, uint3 iyzw) : x(ix), y(iyzw.x), z(iyzw.y), w(iyzw.z) {}
		uint4(float4 i) : x(static_cast<uint>(i.x)), y(static_cast<uint>(i.y)), z(static_cast<uint>(i.z)), w(static_cast<uint>(i.w)) {}
		uint4(const uint4&) = default;
		uint4& operator= (const uint4&) = default;

		uint4 operator-(uint4 i) { return uint4{ x - i.x, y - i.y, z - i.z, w - i.w }; }
		uint4 operator+(uint4 i) { return uint4{ x + i.x, y + i.y, z + i.z, w + i.w }; }
		uint4 operator*(uint4 i) { return uint4{ x * i.x, y * i.y, z * i.z, w * i.w }; }
		uint4 operator/(uint4 i) { return uint4{ x / i.x, y / i.y, z / i.z, w / i.w }; }
		bool operator==(uint4 i) { return x == i.x && x == i.y && z == i.z && w == i.w; }
	};

	enum class Format
	{
		UNKNOW,
		RGBA_16_FLOAT,
		RGBA_32_FLOAT,
		R_16_FLOAT,
		R_32_FLOAT,
		R_32_UINT,
		RGBA_32_UINT
	};

	template<typename data_type> struct as_format { static constexpr Format format = Format::UNKNOW; };
	template<> struct as_format<float> { static constexpr Format format = Format::R_32_FLOAT; };
	template<> struct as_format<float4> { static constexpr Format format = Format::RGBA_32_FLOAT; };
	template<> struct as_format<uint> { static constexpr Format format = Format::R_32_UINT; };
	template<> struct as_format<uint4> { static constexpr Format format = Format::RGBA_32_UINT; };
	*/
}