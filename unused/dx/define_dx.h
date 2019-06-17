#pragma once
#include "../graphic/format.h"
#include <DirectXMath.h>
#include <dxgi.h>
namespace {
	using namespace PO::Graphic;
}

namespace PO::Dx
{

	using matrix = DirectX::XMMATRIX;
	using matrix_par_1 = DirectX::FXMMATRIX;
	using matrix_par_last = DirectX::CXMMATRIX;
	using vector = DirectX::XMVECTOR;
	using vector_par_1_3 = DirectX::FXMVECTOR;
	using vector_par_4 = DirectX::GXMVECTOR;
	using vector_par_5_6 = DirectX::HXMVECTOR;
	using vector_par_7 = DirectX::CXMVECTOR;

	inline matrix matrix_indentity() noexcept { return DirectX::XMMatrixIdentity(); }
	inline matrix inverse(matrix_par_1 v) noexcept { return DirectX::XMMatrixInverse(nullptr, v); }
	inline matrix inverse(matrix_par_1 v, vector& out_v) noexcept { return DirectX::XMMatrixInverse(&out_v, v); }

	
	inline vector length4(vector_par_1_3 v) noexcept { return DirectX::XMVector4Length(v); }
	inline float length4f(vector_par_1_3 v) noexcept { return DirectX::XMVectorGetX(length4(v)); }
	inline vector operator* (vector_par_1_3 v1, vector_par_1_3 v2) noexcept { return DirectX::operator*(v1, v2); }
	inline vector operator* (float v1, vector_par_1_3 v2) noexcept { return DirectX::operator*(v1, v2); }
	inline vector operator* (vector_par_1_3 v1, float v2) noexcept { return DirectX::operator*(v1, v2); }
	inline vector operator/(vector_par_1_3 v1, vector_par_1_3 v2) noexcept { return DirectX::operator/(v1, v2); }
	inline float to_x(vector_par_1_3 v) noexcept { return DirectX::XMVectorGetX(v); }
	inline float to_y(vector_par_1_3 v) noexcept { return DirectX::XMVectorGetY(v); }
	inline float to_z(vector_par_1_3 v) noexcept { return DirectX::XMVectorGetZ(v); }
	inline float to_w(vector_par_1_3 v) noexcept { return DirectX::XMVectorGetW(v); }
	inline vector set_w(vector_par_1_3 v, float v2) noexcept { return DirectX::XMVectorSetW(v, v2); }
	inline vector operator+(vector_par_1_3 v, vector_par_1_3 v2) noexcept { return DirectX::operator+(v, v2); }
	inline vector dot4(vector_par_1_3 v, vector_par_1_3 v2)noexcept { return DirectX::XMVector4Dot(v, v2); }
	inline vector dot3(vector_par_1_3 v, vector_par_1_3 v2)noexcept { return DirectX::XMVector3Dot(v, v2); }
	inline vector cross3(vector_par_1_3 v, vector_par_1_3 v2)noexcept { return DirectX::XMVector3Cross(v, v2); }
	inline vector normalize3(vector_par_1_3 v) noexcept { return DirectX::XMVector3Normalize(v); }
	inline vector normalize4(vector_par_1_3 v) noexcept { return DirectX::XMVector4Normalize(v); }

	inline vector to_vector(const float2& i) { return DirectX::XMLoadFloat2(reinterpret_cast<const DirectX::XMFLOAT2*>(&i)); }
	inline vector to_vector(const float3& i) { return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&i)); }
	inline vector to_vector(const float4& i) { return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&i)); }
	inline vector to_vector(const float& i) { return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&i)); }

	inline vector to_vector(const float2a& i) { return DirectX::XMLoadFloat2A(reinterpret_cast<const DirectX::XMFLOAT2A*>(&i)); }
	inline vector to_vector(const float3a& i) { return DirectX::XMLoadFloat3A(reinterpret_cast<const DirectX::XMFLOAT3A*>(&i)); }
	inline vector to_vector(const float4a& i) { return DirectX::XMLoadFloat4A(reinterpret_cast<const DirectX::XMFLOAT4A*>(&i)); }

	inline float2 to_f2(float2& output, vector_par_1_3 input) { DirectX::XMStoreFloat2(reinterpret_cast<DirectX::XMFLOAT2*>(&output), input); }
	inline float2 to_f2(vector_par_1_3 input) { float2 tem; to_f2(tem, input); return tem; }

	inline float3 to_f3(float3& output, vector_par_1_3 input) { DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&output), input); }
	inline float3 to_f3(vector_par_1_3 input) { float3 tem; to_f3(tem, input); return tem; }

	inline float4 to_f4(float4& output, vector_par_1_3 input) { DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&output), input); }
	inline float4 to_f4(vector_par_1_3 input) { float4 tem; to_f4(tem, input); return tem; }

	inline float2a to_f2a(float2a& output, vector_par_1_3 input) { DirectX::XMStoreFloat2A(reinterpret_cast<DirectX::XMFLOAT2A*>(&output), input); }
	inline float2a to_f2a(vector_par_1_3 input) { float2a tem; to_f2a(tem, input); return tem; }

	inline float3a to_f3a(float3a& output, vector_par_1_3 input) { DirectX::XMStoreFloat3A(reinterpret_cast<DirectX::XMFLOAT3A*>(&output), input); }
	inline float3a to_f3a(vector_par_1_3 input) { float3a tem; to_f3a(tem, input); return tem; }

	inline float4a to_f4a(float4a& output, vector_par_1_3 input) { DirectX::XMStoreFloat4A(reinterpret_cast<DirectX::XMFLOAT4A*>(&output), input); }
	inline float4a to_f4a(vector_par_1_3 input) { float4a tem; to_f4a(tem, input); return tem; }

	inline matrix to_matrix(const float4x4& input) { return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&input)); }
	inline matrix to_matrix(const float4x4a& input) { return DirectX::XMLoadFloat4x4A(reinterpret_cast<const DirectX::XMFLOAT4X4A*>(&input)); }

	inline void to_f44(float4x4& output, matrix_par_1 input) { return DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&output), input); }
	inline float4x4 to_f44(matrix_par_1 input) { float4x4 tem; to_f44(tem, input); return tem; }

	inline void to_f44a(float4x4& output, matrix_par_1 input) { return DirectX::XMStoreFloat4x4A(reinterpret_cast<DirectX::XMFLOAT4X4A*>(&output), input); }
	inline float4x4a to_f44a(matrix_par_1 input) { float4x4a tem; to_f44a(tem, input); return tem; }

	/*
	struct float2
	{
		float x, y;

		float2() : float2(0.0f, 0.0f) {}
		float2(float m_x, float m_y) : x(m_x), y(m_y) {}
		float2(const float2& f) = default;
		explicit float2(vector v) { DirectX::XMStoreFloat2(reinterpret_cast<DirectX::XMFLOAT2*>(this), v); }
		float2& operator=(const float2& f) = default;
		operator vector() const noexcept { return DirectX::XMLoadFloat2(reinterpret_cast<const DirectX::XMFLOAT2*>(this)); }
	};

	struct alignas(16) float2a : float2
	{
		using float2::float2;
		explicit float2a(vector v) { DirectX::XMStoreFloat2A(reinterpret_cast<DirectX::XMFLOAT2A*>(this), v); }
		operator vector() const noexcept { return DirectX::XMLoadFloat2A(reinterpret_cast<const DirectX::XMFLOAT2A*>(this)); }
	};

	struct float3
	{
		float x, y, z;
		float3(float x, float y, float z) noexcept: x(x), y(y), z(z) {}
		float3() noexcept : float3(0.0f, 0.0f, 0.0f) {}
		float3(float2 f, float t =0.0f) noexcept : float3(f.x, f.y, t) {}
		float3(float t, float2 f) noexcept : float3(t, f.x, f.y) {}
		float3(const float3&) = default;
		explicit float3(vector v) noexcept { DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(this), v); }
		float3& operator= (const float3&) = default;
		operator vector() const noexcept { return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(this)); }
	};

	struct alignas(16) float3a : float3
	{
		using float3::float3;
		explicit float3a(vector v) noexcept { DirectX::XMStoreFloat3A(reinterpret_cast<DirectX::XMFLOAT3A*>(this), v); }
		operator vector() const noexcept { return DirectX::XMLoadFloat3A(reinterpret_cast<const DirectX::XMFLOAT3A*>(this)); }
	};

	struct float4
	{
		float x, y, z, w;
		float4() : float4(0.0f, 0.0f, 0.0f, 0.0f) {}
		float4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
		float4(float x, float y, float2 w) noexcept : float4(x, y, w.x, w.y) {}
		float4(float x, float2 w, float y) noexcept : float4(x, w.x, w.y, y) {}
		float4(float2 x, float w, float y) noexcept : float4(x.x, x.y, w, y) {}
		float4(float t, float3 p) noexcept : float4(t, p.x, p.y, p.z) {}
		float4(float3 t, float p) noexcept : float4(t.x, t.y, t.z, p) {}
		float4(const float4&) = default;
		explicit float4(vector v) noexcept { DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), v); }
		operator vector() const noexcept { return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(this)); }
	};

	struct alignas(16) float4a : float4
	{
		using float4::float4;
		explicit float4a(vector v) noexcept { DirectX::XMStoreFloat4A(reinterpret_cast<DirectX::XMFLOAT4A*>(this), v); }
		operator vector() const noexcept { return DirectX::XMLoadFloat4A(reinterpret_cast<const DirectX::XMFLOAT4A*>(this)); }
	};

	struct uint2
	{
		uint32_t x, y;
		uint2(uint32_t x, uint32_t y) : x(x), y(y) {}
		operator vector() const noexcept { return DirectX::XMLoadUInt2(reinterpret_cast<const DirectX::XMUINT2*>(this)); }
		explicit uint2(vector v) noexcept { DirectX::XMStoreUInt2(reinterpret_cast<DirectX::XMUINT2*>(this), v); }
	};

	struct uint3
	{
		uint32_t x, y, z;
		uint3(uint32_t x, uint32_t y, uint32_t z) : x(x), y(y), z(z) {}
		operator vector() const noexcept { return DirectX::XMLoadUInt3(reinterpret_cast<const DirectX::XMUINT3*>(this)); }
		explicit uint3(vector v) noexcept { DirectX::XMStoreUInt3(reinterpret_cast<DirectX::XMUINT3*>(this), v); }
	};

	struct float4x4
	{
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			float m[4][4];
		};

		float4x4(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33)
			: _11(m00), _12(m01), _13(m02), _14(m03),
			_21(m10), _22(m11), _23(m12), _24(m13),
			_31(m20), _32(m21), _33(m22), _34(m23),
			_41(m30), _42(m31), _43(m32), _44(m33) {}

		float4x4() {}

		explicit float4x4(matrix m) noexcept { DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(this), m); }
		operator matrix() const noexcept { return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(this)); }
	};

	struct alignas(16) float4x4a : float4x4
	{
		using float4x4::float4x4;
		float4x4a() = default;
		explicit float4x4a(matrix m) noexcept { DirectX::XMStoreFloat4x4A(reinterpret_cast<DirectX::XMFLOAT4X4A*>(this), m); }
		operator matrix() const noexcept { return DirectX::XMLoadFloat4x4A(reinterpret_cast<const DirectX::XMFLOAT4X4A*>(this)); }
	};

	struct tex_sample
	{
		uint32_t count = 1;
		uint32_t quality = 0;
	};
	*/

	/*
	using float4 = DirectX::XMFLOAT4;
	using float4a = DirectX::XMFLOAT4A;
	using float4x4 = DirectX::XMFLOAT4X4;
	using float4x4a = DirectX::XMFLOAT4X4A;
	using int32_t2 = DirectX::XMINT2;
	using int32_t3 = DirectX::XMINT3;
	using int32_t4 = DirectX::XMINT4;
	using uint32_t2 = DirectX::XMUINT2;
	using uint32_t3 = DirectX::XMUINT3;
	using uint32_t4 = DirectX::XMUINT4;
	using uint = uint32_t;
	using uint2 = uint32_t2;
	using uint3 = uint32_t3;
	using uint4 = uint32_t4;
	using matrix = DirectX::XMMATRIX;
	using matrix_ref = DirectX::CXMMATRIX;
	using vector = DirectX::XMVECTOR;
	*/
}