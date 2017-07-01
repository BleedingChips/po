#pragma once
#include "../po_dxgi/dxgi_define.h"
#include <DirectXMath.h>
namespace PO
{
	namespace Dx
	{
		using float2 = DirectX::XMFLOAT2;
		using float3 = DirectX::XMFLOAT3;
		using float4 = DirectX::XMFLOAT4;
		using float4x4 = DirectX::XMFLOAT4X4;
		using int32_t2 = DirectX::XMINT2;
		using int32_t3 = DirectX::XMINT3;
		using int32_t4 = DirectX::XMINT4;
		using uint32_t2 = DirectX::XMUINT2;
		using uint32_t3 = DirectX::XMUINT3;
		using uint32_t4 = DirectX::XMUINT4;
		using matrix = DirectX::XMMATRIX;
		using vector = DirectX::XMVECTOR;

		struct tex_sample
		{
			UINT count = 1;
			UINT quality = 0;
		};

	}

	namespace DXGI
	{
		template<typename T>
		struct size_struct
		{
			static constexpr size_t size = sizeof(T);
			static constexpr size_t align = alignof(T);
		};

		template<typename T> struct data_format;
		template<> struct data_format<Dx::float2> : size_struct<Dx::float2>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
		};
		template<> struct data_format<Dx::float3> : size_struct<Dx::float3>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		};
		template<> struct data_format<Dx::float4> : size_struct<Dx::float4>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
		};
		template<> struct data_format<Dx::int32_t2> : size_struct<Dx::int32_t2>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32_SINT;
		};
		template<> struct data_format<Dx::int32_t3> : size_struct<Dx::int32_t3>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_SINT;
		};
		template<> struct data_format<Dx::int32_t4> : size_struct<Dx::int32_t4>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_SINT;
		};
		template<> struct data_format<Dx::uint32_t2> : size_struct<Dx::uint32_t2>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32_UINT;
		};
		template<> struct data_format<Dx::uint32_t3> : size_struct<Dx::uint32_t3>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_UINT;
		};
		template<> struct data_format<Dx::uint32_t4> : size_struct<Dx::uint32_t4>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_UINT;
		};
		template<> struct data_format<Dx::float4x4> : size_struct<Dx::float4x4>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		};
	}
}

inline PO::Dx::float2 operator+(PO::Dx::float2 i, PO::Dx::float2 o) {
	return PO::Dx::float2(i.x + o.x, i.y + o.y);
}

inline PO::Dx::float3 operator+(PO::Dx::float3 i, PO::Dx::float3 o) {
	return PO::Dx::float3(i.x + o.x, i.y + o.y, i.z + o.z);
}

inline PO::Dx::float3 operator*(PO::Dx::float3 i, float o) {
	return PO::Dx::float3(i.x * o, i.y * o, i.z + o);
}

inline PO::Dx::float3 operator-(PO::Dx::float3 i, float o) {
	return PO::Dx::float3(i.x - o, i.y - o, i.z - o);
}