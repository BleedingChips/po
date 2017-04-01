#pragma once
#include <ostream>
#include "dx_type.h"
namespace PO
{
	namespace Dx
	{

		// for debug
		inline std::ostream& operator<<(std::ostream& o, const float2& m)
		{
			return o << "{" << m.x << "," << m.y << "}";
		}

		inline std::ostream& operator<<(std::ostream& o, const float3& m)
		{
			return o << "{" << m.x << "," << m.y << "," << m.z << "}";
		}

		inline std::ostream& operator<<(std::ostream& o, const float4& m)
		{
			return o << "{" << m.x << "," << m.y << "," << m.z << ","<< m.w << "}";
		}

		inline std::ostream& operator<<(std::ostream& o, const float4x4& m)
		{
			bool need_add = false;
			o << "{";
			for (size_t i =0 ; i < 4; ++i)
			{
				if (!need_add)
					need_add = true;
				else
					o << ",";
				o << DirectX::XMFLOAT4{ m.m[i][0],m.m[i][1],m.m[i][2],m.m[i][3] };
			}
			return o << "}";
		}

		struct quaternions
		{
			struct alignas(alignof(DirectX::XMVECTOR)) quaternions_tem
			{
				DirectX::XMVECTOR real;
				DirectX::XMVECTOR imaginary;
			};
			float real;
			float3 imaginary;
		public:
			//quaternions operator* (const quaternions& m);
			//float3 tarnfer(const float3& m);
			quaternions(const quaternions_tem& qt);
			quaternions(const quaternions&) = default;
			quaternions(float angle, float3 nor);
		};

		inline std::ostream& operator<<(std::ostream& o, const quaternions& m)
		{
			return o << "{" << m.real << "," << m.imaginary << "}";
		}
	}
}