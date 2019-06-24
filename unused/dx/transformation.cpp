#include "transformation.h"
namespace PO::Dx
{
	float quaternions::length() const noexcept { return length4f(to_vector(storage)); }
	vector quaternions::conjugate() const noexcept {
		using namespace DirectX;
		return vector{-1.0, -1.0, -1.0, 1.0} *to_vector(storage);
	}

	vector quaternions::inverrse() const noexcept
	{
		auto tem = to_vector(storage);
		return vector{ -1.0, -1.0, -1.0, 1.0 } *tem / length4(tem);
	}

	vector quaternions::operator*(const quaternions& qt) const noexcept
	{
		auto storage_tem = to_vector(storage);
		auto storage_tem_qt = to_vector(qt.storage);
		float a = storage.w, b = qt.storage.w;
		vector s1 = storage_tem, s2 = storage_tem_qt;
		auto tem = s1 * a + b * s2 + cross3(s1, s2);
		return{ normalize4(set_w(
			tem,
			a * b - to_x(dot3(storage_tem, storage_tem_qt)))
		) };
	}

	vector rotation_axis::to_quaternions() const noexcept
	{
		float radian = storage.w / 180.0f * to_x(DirectX::g_XMPi);
		auto ver = normalize3(to_vector(storage)) * vector { sinf(radian / 2.0f) };
		return set_w(ver, cosf(radian / 2.0f));
	}

	transformation3D::transformation3D()
	{
		to_f44a(m_matrix, DirectX::XMMatrixIdentity());
	}
	transformation3D::transformation3D(float3a position, float3a rorate)
	{
		to_f44a(m_matrix, DirectX::XMMatrixIdentity());
		m_matrix._11 /= rorate.x;
		m_matrix._22 /= rorate.y;
		m_matrix._33 /= rorate.y;
		m_matrix._14 = position.x;
		m_matrix._24 = position.y;
		m_matrix._34 = position.z;
	}

	matrix transformation3D::inverse() const
	{
		return Dx::inverse(to_matrix(m_matrix));
	}


}



/*
namespace PO
{
	namespace Dx
	{
		using namespace DirectX;
		float quaternions_template::length() const {
			return DirectX::XMVectorGetX(DirectX::XMVector4Length(store));
		}

		quaternions_template quaternions_template::conjugate() const {
			using namespace DirectX;
			return { DirectX::operator*(store , XMVECTOR{ -1.0, -1.0, -1.0, 1.0 }) };
		}

		quaternions_template quaternions_template::inverrse() const {
			using namespace DirectX;
			return { DirectX::operator/ (conjugate().store ,length()) };
		}


		quaternions::quaternions(const quaternions_template& qt)
		{
			using namespace DirectX;
			XMStoreFloat4(&store, qt.store);
		}

		quaternions::operator quaternions_template() const
		{
			using namespace DirectX;
			return{XMLoadFloat4(&store)};
		}

		float4x4 quaternions::to_float4x4(float3 s, float3 p) const
		{
			float xx = store.x * store.x, yy = store.y * store.y, zz = store.z * store.z, ww = store.w * store.w;
			float xy = store.x * store.y, xz = store.x * store.z, xw = store.x * store.w,
				yz = store.y * store.z, yw = store.y * store.w, zw = store.z * store.w;
			return{
				(2.0f * (xx + ww) - 1.0f) * s.x, 2.0f * (xy - zw) * s.x, 2.0f * (xz + yw) * s.x, 0.0f,
				2.0f * (xy + zw) * s.y, (2.0f * (yy + ww) - 1.0f) * s.y, 2.0f * (yz - xw) * s.y, 0.0f,
				2.0f * (xz - yw) * s.z, 2.0f * (yz + xw) * s.z, (2.0f * (zz + ww) - 1.0f) * s.z, 0.0f,
				p.x, p.y, p.z, 1.0
			};
		}

		DirectX::XMMATRIX quaternions::to_XMMATRIX(float3 s, float3 p) const
		{
			float xx = store.x * store.x, yy = store.y * store.y, zz = store.z * store.z, ww = store.w * store.w;
			float xy = store.x * store.y, xz = store.x * store.z, xw = store.x * store.w,
				yz = store.y * store.z, yw = store.y * store.w, zw = store.z * store.w;
			return{
				(2.0f * (xx + ww) - 1.0f) * s.x, 2.0f * (xy - zw) * s.x, 2.0f * (xz + yw) * s.x, 0.0f,
				2.0f * (xy + zw) * s.y, (2.0f * (yy + ww) - 1.0f) * s.y, 2.0f * (yz - xw) * s.y, 0.0f,
				2.0f * (xz - yw) * s.z, 2.0f * (yz + xw) * s.z, (2.0f * (zz + ww) - 1.0f) * s.z, 0.0f,
				p.x, p.y, p.z, 1.0
			};
		}

		float3 quaternions::rate(float3 v) const {
			quaternions_template input{ { v.x, v.y, v.z, 0.0 } };
			quaternions_template rat = *this;
			auto result = rat * input * rat.inverrse();
			float3 re;
			DirectX::XMStoreFloat3(&re, result.store);
			return re;
		}

		quaternions& quaternions::operator=(const quaternions_template& qt) {
			DirectX::XMStoreFloat4(&store, qt.store);
			return *this;
		}

		eulerian_angle::operator quaternions_template() const {
			float cx = cosf(angle.x / 2.0f), cy = cosf(angle.y / 2.0f), cz = cosf(angle.z / 2.0f);
			float sx = sinf(angle.x / 2.0f), sy = sinf(angle.y / 2.0f), sz = sinf(angle.z / 2.0f);
			return{
					sx * cy * cz - cx * sy * sz,
					cx * sy * cz + sx * cy * sz,
					cx * cy * sz - sx * sy * cz,
					cx * cy * cz + sx * sy * sz // s
			};
		}

		rotation_axis::operator quaternions_template() const
		{
			using namespace DirectX;
			float radian = angle / 180.0f * 3.141592653f;
			auto ver = XMLoadFloat3(&axis);
			ver = DirectX::operator*(XMVector3Normalize(ver) , sinf(radian / 2.0f));
			return{ XMVectorSetW(ver, cosf(radian / 2.0f)) };
		}

		quaternions_template quaternions_template::operator*(const quaternions_template& qt) const {
			using namespace DirectX;
			float a = XMVectorGetW(store), b = XMVectorGetW(qt.store);
			return{ XMVector4Normalize( XMVectorSetW(
				DirectX::operator+(DirectX::operator+(DirectX::operator*(a , qt.store), DirectX::operator*(b , store)) , XMVector3Cross(store, qt.store)),
				a * b - XMVectorGetX(XMVector3Dot(store, qt.store)) )
			) };
		}

		eulerian_angle::eulerian_angle(const quaternions_template& q){
			using namespace DirectX;
			float4 data;
			XMStoreFloat4(&data, q.store);
			angle = {
				atan2f(2.0f * (data.w * data.x + data.y * data.z), (1.0f - 2.0f * (data.x * data.x + data.y * data.y))),
				asinf(2.0f * (data.w * data.y - data.z * data.x)),
				atan2f(2.0f * (data.w * data.z + data.x * data.y), (1.0f - 2.0f * (data.y * data.y + data.z * data.z)))
			};
		}


		matrix transfer3D::inverse_matrix() const
		{
			return DirectX::XMMatrixInverse(nullptr, qua.to_XMMATRIX(sca, poi));
		}

		float4x4 transfer3D::inverse_float4x4() const
		{
			float4x4 tem;
			DirectX::XMStoreFloat4x4(&tem, inverse_matrix());
			return tem;
		}

	}
}

std::ostream& operator<<(std::ostream& o, const PO::float2& m)
{
	return o << "{" << m.x << "," << m.y << "}";
}

std::ostream& operator<<(std::ostream& o, const PO::float3& m)
{
	return o << "{" << m.x << "," << m.y << "," << m.z << "}";
}

std::ostream& operator<<(std::ostream& o, const PO::float4& m)
{
	return o << "{" << m.x << "," << m.y << "," << m.z << "," << m.w << "}";
}

std::ostream& operator<<(std::ostream& o, const PO::float4x4& m)
{
	bool need_add = false;
	o << "{";
	for (size_t i = 0; i < 4; ++i)
	{
		if (!need_add)
			need_add = true;
		else
			o << ",";
		o << DirectX::XMFLOAT4{ m.m[i][0],m.m[i][1],m.m[i][2],m.m[i][3] };
	}
	return o << "}";
}

std::ostream& operator<<(std::ostream& o, const PO::Dx::quaternions& m)
{
	return o << "{" << m.store.w << ","<< PO::float3 {m.store.x, m.store.y, m.store.z } << "}";
}
*/