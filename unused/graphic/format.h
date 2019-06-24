#pragma once
#include <stdint.h>
namespace PO ::Graphic
{
	enum class FormatPixel : uint32_t
	{
		UNKNOW,
		F32,
		F16,
		UI16,
		UI32,
		RGB_F32,
		RGBA_F32,
		RGBA_F16,
		RGBA_UI8,
		RGBA_U8,
		RGBA_I8,
	};

	enum class FormatRT : uint32_t
	{
		RGB_F32 = static_cast<uint32_t>(FormatPixel::RGB_F32),
		RGBA_F32 = static_cast<uint32_t>(FormatPixel::RGBA_F32),
		RGBA_F16 = static_cast<uint32_t>(FormatPixel::RGBA_F16),
	};

	enum class DSFormat
	{

	};

	inline FormatPixel translate_FormatRT(FormatRT input) noexcept { return static_cast<FormatPixel>(static_cast<uint32_t>(input)); }

	uint8_t calculate_pixel_size(FormatPixel format);
	//uint8_t calculate_pixel_size(FormatIndex format);

	struct float2
	{
		float x, y;

		float2() : float2(0.0f, 0.0f) {}
		float2(float m_x, float m_y) : x(m_x), y(m_y) {}
		float2(const float2& f) = default;
		//explicit float2(vector v) { DirectX::XMStoreFloat2(reinterpret_cast<DirectX::XMFLOAT2*>(this), v); }
		float2& operator=(const float2& f) = default;
		//operator vector() const noexcept { return DirectX::XMLoadFloat2(reinterpret_cast<const DirectX::XMFLOAT2*>(this)); }
	};

	struct alignas(16) float2a : float2
	{
		using float2::float2;
		//explicit float2a(vector v) { DirectX::XMStoreFloat2A(reinterpret_cast<DirectX::XMFLOAT2A*>(this), v); }
		//operator vector() const noexcept { return DirectX::XMLoadFloat2A(reinterpret_cast<const DirectX::XMFLOAT2A*>(this)); }
	};

	struct float3
	{
		float x, y, z;
		float3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
		float3() noexcept : float3(0.0f, 0.0f, 0.0f) {}
		float3(float2 f, float t = 0.0f) noexcept : float3(f.x, f.y, t) {}
		float3(float t, float2 f) noexcept : float3(t, f.x, f.y) {}
		float3(const float3&) = default;
		//explicit float3(vector v) noexcept { DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(this), v); }
		float3& operator= (const float3&) = default;
		//operator vector() const noexcept { return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(this)); }
	};

	struct alignas(16) float3a : float3
	{
		using float3::float3;
		//explicit float3a(vector v) noexcept { DirectX::XMStoreFloat3A(reinterpret_cast<DirectX::XMFLOAT3A*>(this), v); }
		//operator vector() const noexcept { return DirectX::XMLoadFloat3A(reinterpret_cast<const DirectX::XMFLOAT3A*>(this)); }
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
		//explicit float4(vector v) noexcept { DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), v); }
		//operator vector() const noexcept { return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(this)); }
	};

	struct alignas(16) float4a : float4
	{
		using float4::float4;
		//explicit float4a(vector v) noexcept { DirectX::XMStoreFloat4A(reinterpret_cast<DirectX::XMFLOAT4A*>(this), v); }
		//operator vector() const noexcept { return DirectX::XMLoadFloat4A(reinterpret_cast<const DirectX::XMFLOAT4A*>(this)); }
	};

	using uint = uint32_t;

	struct uint2
	{
		uint32_t x, y;
		uint2(uint32_t x = 0, uint32_t y = 0) : x(x), y(y) {}
		uint2 operator / (uint32_t input) const noexcept { return {x / input, y /input}; }
		uint2& operator /= (uint32_t input) noexcept { x /= input; y /= input; return *this; }
		bool operator==(uint2 input) { return x == input.x && y == input.y; }
		//operator vector() const noexcept { return DirectX::XMLoadUInt2(reinterpret_cast<const DirectX::XMUINT2*>(this)); }
		//explicit uint2(vector v) noexcept { DirectX::XMStoreUInt2(reinterpret_cast<DirectX::XMUINT2*>(this), v); }
	};

	struct int2
	{
		int32_t x, y;
		int2(int32_t x, int32_t y) : x(x), y(y) {}
	};

	struct uint3
	{
		uint32_t x, y, z;
		uint3(uint32_t x = 0, uint32_t y =0 , uint32_t z =0) : x(x), y(y), z(z) {}
		uint3(uint2 xy, uint32_t z = 0) : x(xy.x), y(xy.y), z(z) {}
		//operator vector() const noexcept { return DirectX::XMLoadUInt3(reinterpret_cast<const DirectX::XMUINT3*>(this)); }
		//explicit uint3(vector v) noexcept { DirectX::XMStoreUInt3(reinterpret_cast<DirectX::XMUINT3*>(this), v); }
	};

	uint8_t tex_dimension(uint3);

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

		//explicit float4x4(matrix m) noexcept { DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(this), m); }
		//operator matrix() const noexcept { return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(this)); }
	};

	struct alignas(16) float4x4a : float4x4
	{
		using float4x4::float4x4;
		float4x4a() = default;
		//explicit float4x4a(matrix m) noexcept { DirectX::XMStoreFloat4x4A(reinterpret_cast<DirectX::XMFLOAT4X4A*>(this), m); }
		//operator matrix() const noexcept { return DirectX::XMLoadFloat4x4A(reinterpret_cast<const DirectX::XMFLOAT4X4A*>(this)); }
	};

	template<typename input_type> struct as_format{static constexpr FormatPixel value = FormatPixel::UNKNOW;};
	template<> struct as_format<uint16_t> {static constexpr FormatPixel value = FormatPixel::UI16;};
	template<> struct as_format<float3> { static constexpr FormatPixel value = FormatPixel::RGB_F32; };
}