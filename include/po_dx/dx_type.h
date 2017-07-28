#pragma once
#include "../po_dxgi/dxgi_define.h"
#include "../po/tool/tool.h"
#include <DirectXMath.h>
#include <fstream>
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
		using uint = uint32_t;
		using uint2 = uint32_t2;
		using uint3 = uint32_t3;
		using uint4 = uint32_t4;
		using matrix = DirectX::XMMATRIX;
		using matrix_ref = DirectX::CXMMATRIX;
		using vector = DirectX::XMVECTOR;

		struct tex_sample
		{
			UINT count = 1;
			UINT quality = 0;
		};

		class shader_binary
		{
			void* data = nullptr;
			std::streamsize size = 0;
		public:
			shader_binary(std::fstream& f);
			~shader_binary();
			operator const void* () const { return data; }
			operator UINT() const { return static_cast<UINT>(size); }
		};

		template<typename T, size_t count> class aligned_array
		{
			static constexpr size_t single_size = (sizeof(T) % 16 == 0) ? sizeof(T) : (sizeof(T) - (sizeof(T) % 16) + 16);
			char data[single_size * count];
		public:
			T& operator[](size_t k) { return *reinterpret_cast<T*>(data + single_size * k); }
			const T& operator[](size_t k) const { return *reinterpret_cast<const T*>(data + single_size * k); }


			aligned_array& operator=(const aligned_array& aa)
			{
				for (size_t i = 0; i < count; ++i)
					(*this)[i] = aa[i];
				return *this;
			}

			aligned_array& operator=(aligned_array&& aa)
			{
				for (size_t i = 0; i < count; ++i)
					(*this)[i] = std::move(aa[i]);
				return *this;
			}

			aligned_array& operator=(const std::array<T, count> &aa)
			{
				for (size_t i = 0; i < count; ++i)
					(*this)[i] = aa[i];
				return *this;
			}



			aligned_array& operator=(std::array<T, count>&& aa)
			{
				for (size_t i = 0; i < count; ++i)
					(*this)[i] = std::move(aa[i]);
				return *this;
			}

			aligned_array()
			{
				for (size_t i = 0; i < count; ++i)
				{
					new(data + single_size * i) T{};
				}
			}
			~aligned_array()
			{
				for (size_t i = 0; i < count; ++i)
					this->operator[](i).~T();
			}
			aligned_array(const std::array<T, count>& il)
			{
				size_t curent_size = 0;
				for (; curent_size < count; ++curent_size)
					new(data + single_size * curent_size) T{ il[curent_size] };
			}
			aligned_array(std::array<T, count>&& il)
			{
				size_t curent_size = 0;
				for (; curent_size < count; ++curent_size)
					new(data + single_size * curent_size) T{ std::move(il[curent_size]) };
			}
			aligned_array(const aligned_array& aa)
			{
				for (size_t i = 0; i < count; ++i)
				{
					new(data + single_size * i) T{ aa[i] };
				}
			}
			aligned_array(aligned_array&& aa)
			{
				for (size_t i = 0; i < count; ++i)
					new(data + single_size * i) T{ std::move(aa[i]) };
			}
			aligned_array(std::initializer_list<T> il)
			{
				size_t curent_size = 0;
				for (auto& p : il)
				{
					if (curent_size < count)
					{
						new(data + single_size * curent_size) T{ p };
						++curent_size;
					}
					else break;
				}
				for (; curent_size < count; ++curent_size)
					new(data + single_size * curent_size) T{ };
			}

		};

		namespace Implement
		{
			template<size_t last, size_t current> struct shader_storage_start_size
			{
				static constexpr size_t fix_size = (last % 4 == 0) ? last : last + 4 - last % 4;

				static constexpr size_t size =
					(last % 16 == 0) ?
					last :
					(
					((fix_size % 16) + current > 16) ?
						(fix_size - fix_size % 16 + 16)
						: fix_size
						)
					;
			};

			template<size_t last, typename ...T> struct shader_storage_count_size
			{
				static constexpr size_t size = (last == 0) ? 1 : last;
			};

			template<size_t last, typename T, typename ...K> struct shader_storage_count_size<last, T, K...>
			{
				static constexpr size_t size = shader_storage_count_size<shader_storage_start_size<last, sizeof(T)>::size + sizeof(T), K...>::size;
			};

			template<size_t last, typename ...K> struct shader_storage_element_handle
			{
				static void construction(char*) {}
				static void destruction(char*) {}
				static void equal(char*, const char*) {}
				static void equal(char*, char*&&) {}
				static void construct_sample_type(char* da, const char* src) {}
				static void construct_sample_type(char* da, char*&& src) {}
			};

			template<size_t last, typename T, typename ...K> struct shader_storage_element_handle<last, T, K...>
			{
				template<typename P, typename ...AK> static void construction(char* da, P&& p, AK&& ...ak)
				{
					new (da + shader_storage_start_size<last, sizeof(T)>::size) T(std::forward<P>(p));
					shader_storage_element_handle<shader_storage_start_size<last, sizeof(T)>::size + sizeof(T), K...>::construction(da, std::forward<AK>(ak)...);
				}
				static void construction(char* da)
				{
					new (da + shader_storage_start_size<last, sizeof(T)>::size) T();
					shader_storage_element_handle<shader_storage_start_size<last, sizeof(T)>::size + sizeof(T), K...>::construction(da);
				}

				static void destruction(char* da)
				{
					reinterpret_cast<T*>(da + shader_storage_start_size<last, sizeof(T)>::size)->~T();
					shader_storage_element_handle<shader_storage_start_size<last, sizeof(T)>::size + sizeof(T), K...>::destruction(da);
				}

				static void construct_sample_type(char* da, const char* src) {
					new(da + shader_storage_start_size<last, sizeof(T)>::size) T(*reinterpret_cast<const T*>(src + shader_storage_start_size<last, sizeof(T)>::size));
					shader_storage_element_handle<shader_storage_start_size<last, sizeof(T)>::size + sizeof(T), K...>::construct_sample_type(da, src);
				}

				static void construct_sample_type(char* da, char*&& src) {
					new(da + shader_storage_start_size<last, sizeof(T)>::size) T(std::move(*reinterpret_cast<T*>(src + shader_storage_start_size<last, sizeof(T)>::size)));
					shader_storage_element_handle<shader_storage_start_size<last, sizeof(T)>::size + sizeof(T), K...>::construct_sample_type(da, std::move(src));
				}

				static void equal(char* da, const char* src) {
					*reinterpret_cast<T*>(da + shader_storage_start_size<last, sizeof(T)>::size) = (*reinterpret_cast<const T*>(src + shader_storage_start_size<last, sizeof(T)>::size));
					shader_storage_element_handle<shader_storage_start_size<last, sizeof(T)>::size + sizeof(T), K...>::equal(da, src);
				}
				static void equal(char* da, char*&& src) {
					*reinterpret_cast<T*>(da + shader_storage_start_size<last, sizeof(T)>::size) = (std::move(*reinterpret_cast<T*>(src + shader_storage_start_size<last, sizeof(T)>::size)));
					shader_storage_element_handle<shader_storage_start_size<last, sizeof(T)>::size + sizeof(T), K...>::equal(da, std::move(src));
				}
			};

			template<size_t index, size_t last, typename K, typename ...T> struct shader_storage_get
			{
				static constexpr size_t size = shader_storage_get<index - 1, shader_storage_start_size<last, sizeof(K)>::size + sizeof(K), T...>::size;
			};

			template<size_t last, typename K, typename ...T> struct shader_storage_get<0, last, K, T...>
			{
				static constexpr size_t size = shader_storage_start_size<last, sizeof(K)>::size;
			};
		}

		template<typename ...T> class alignas(16) shader_storage
		{
			alignas(16) char data[Implement::shader_storage_count_size<0, T...>::size];
			template<size_t i> using selet_type_t = std::decay_t<decltype(std::get<i>(std::tuple<T...>{})) > ;
			template<size_t i> friend struct shader_storage_get;
		public:
			template<typename ...AK> shader_storage(AK&& ... ak) { Implement::shader_storage_element_handle<0, T...>::construction(data, std::forward<AK>(ak)...); }
			shader_storage(const shader_storage& ss) {
				Implement::shader_storage_element_handle<0, T...>::construct_sample_type(data, ss.data);
			}
			shader_storage(shader_storage&& ss) {
				Implement::shader_storage_element_handle<0, T...>::construct_sample_type(data, std::move(ss.data));
			}

			/*
			template<size_t i> auto get() -> selet_type_t<i>&  {
				//using final_t = TmpCall::call<TmpCall::append<T...>, TmpCall::select_index<std::integral_constant<size_t, i>>, TmpCall::self>;
				return *reinterpret_cast<selet_type_t<i>*>(data + Implement::aligned_storage_get<i, 0, T...>::size);
				//return final_t{};
			}*/
			shader_storage& operator=(const shader_storage& as) {
				Implement::shader_storage_element_handle<0, T...>::equal(data, as.data);
				return *this;
			}
			shader_storage& operator=(shader_storage&& as) {
				Implement::shader_storage_element_handle<0, T...>::equal(data, std::move(as.data));
				return *this;
			}
			~shader_storage() {
				Implement::shader_storage_element_handle<0, T...>::destruction(data);
			}
		};

		template<size_t i> struct shader_storage_get
		{
			template<typename ...T>
			auto operator()(shader_storage<T...>& p) const->std::decay_t<decltype(std::get<i>(std::tuple<T...>{})) > &
			{
				return *reinterpret_cast<std::decay_t<decltype(std::get<i>(std::tuple<T...>{})) > * > (p.data + Implement::shader_storage_get<i, 0, T...>::size);
			}
			template<typename ...T>
			auto operator()(const shader_storage<T...>& p) const-> const std::decay_t<decltype(std::get<i>(std::tuple<T...>{})) > &
			{
				return *reinterpret_cast<const std::decay_t<decltype(std::get<i>(std::tuple<T...>{})) > * > (p.data + Implement::shader_storage_get<i, 0, T...>::size);
			}
		};
	}
}

namespace std
{
	template<size_t i, typename ...T> decltype(auto) get(PO::Dx::shader_storage<T...>& ss) { return PO::Dx::shader_storage_get<i>{}(ss); }
	template<size_t i, typename ...T> decltype(auto) get(const PO::Dx::shader_storage<T...>& ss) { return PO::Dx::shader_storage_get<i>{}(ss); }
}

namespace PO
{
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

inline PO::Dx::float3 operator-(PO::Dx::float3 i, PO::Dx::float3 o) {
	return PO::Dx::float3(i.x - o.x, i.y - o.y, i.z - o.z);
}

inline PO::Dx::float3 operator*(PO::Dx::float3 i, float o) {
	return PO::Dx::float3(i.x * o, i.y * o, i.z + o);
}

inline PO::Dx::float3 operator-(PO::Dx::float3 i, float o) {
	return PO::Dx::float3(i.x - o, i.y - o, i.z - o);
}