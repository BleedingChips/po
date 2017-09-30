#pragma once
#include <d3d11_3.h>
#include "dx11_define.h"
#include <tuple>
#include "../po/tool/auto_adapter.h"
#include "../po_dx/dx_type.h"
#include <optional>

namespace PO
{
	namespace Dx11
	{
		using namespace Dx;

		uint32_t calculate_mipmap_count(uint32_t p, uint32_t mipmap);

		inline uint32_t calculate_mipmap_count(uint32_t2 p, uint32_t mipmap)
		{
			return calculate_mipmap_count(p.x > p.y ? p.x : p.y, mipmap);
		}

		inline uint32_t calculate_mipmap_count(uint32_t3 p, uint32_t mipmap)
		{
			return calculate_mipmap_count(uint32_t2{ p.x > p.y ? p.x : p.y, p.z }, mipmap);
		}

		struct creator
		{
			Win32::com_ptr<ID3D11Device> dev;
			creator(Win32::com_ptr<ID3D11Device> d) : dev(std::move(d)) { assert(dev); }
			creator(const creator&) = default;
		};

		struct shader_resource_view_interface {
			Win32::com_ptr<ID3D11ShaderResourceView> ptr;
			std::tuple<shader_resource_view_interface, uint32_t> operator[](uint32_t i) && {return std::tuple<shader_resource_view_interface, uint32_t>{std::move(*this), i}; }
			std::tuple<const shader_resource_view_interface&, uint32_t> operator[](uint32_t i) const& { return std::tuple<const shader_resource_view_interface&, uint32_t>{*this, i}; }
			operator bool() const { return ptr; }
			shader_resource_view_interface() = default;
			shader_resource_view_interface(Win32::com_ptr<ID3D11ShaderResourceView> p) : ptr(std::move(p)) {}
			shader_resource_view_interface(const shader_resource_view_interface&) = default;
			shader_resource_view_interface(shader_resource_view_interface&&) = default;
			shader_resource_view_interface& operator=(const shader_resource_view_interface&) = default;
			shader_resource_view_interface& operator=(shader_resource_view_interface&&) = default;
		};

		struct unordered_access_view_interface {
			Win32::com_ptr<ID3D11UnorderedAccessView> ptr; uint32_t offset = 0;
			std::tuple<unordered_access_view_interface, uint32_t> operator[](uint32_t i) && {return std::tuple<unordered_access_view_interface, uint32_t>{std::move(*this), i}; }
			std::tuple<const unordered_access_view_interface&, uint32_t> operator[](uint32_t i) const& { return std::tuple<const unordered_access_view_interface&, uint32_t>{*this, i}; }
			operator bool() const { return ptr; }
			unordered_access_view_interface() = default;
			unordered_access_view_interface(Win32::com_ptr<ID3D11UnorderedAccessView> p, uint32_t off = 0) : ptr(std::move(p)), offset(off) {}
			unordered_access_view_interface(const unordered_access_view_interface&) = default;
			unordered_access_view_interface(unordered_access_view_interface&&) = default;
			unordered_access_view_interface& operator=(const unordered_access_view_interface&) = default;
			unordered_access_view_interface& operator=(unordered_access_view_interface&&) = default;
		};

		struct render_target_view_interface {
			Win32::com_ptr<ID3D11RenderTargetView> ptr;
			//std::tuple<render_target_view_interface, uint32_t> operator[](uint32_t i) && {return std::tuple<render_target_view_interface, uint32_t>{std::move(*this), i}; }
			//std::tuple<const render_target_view_interface&, uint32_t> operator[](uint32_t i) const& { return std::tuple<const render_target_view_interface&, uint32_t>{*this, i}; }
			operator bool() const { return ptr; }
			render_target_view_interface() = default;
			render_target_view_interface(Win32::com_ptr<ID3D11RenderTargetView> p) : ptr(std::move(p)) {}
			render_target_view_interface(const render_target_view_interface&) = default;
			render_target_view_interface(render_target_view_interface&&) = default;
			render_target_view_interface& operator=(const render_target_view_interface&) = default;
			render_target_view_interface& operator=(render_target_view_interface&&) = default;
		};

		struct depth_stencil_view_interface {
			Win32::com_ptr<ID3D11DepthStencilView> ptr;
			operator bool() const { return ptr; }
			depth_stencil_view_interface() = default;
			depth_stencil_view_interface(Win32::com_ptr<ID3D11DepthStencilView> p) : ptr(std::move(p)) {}
			depth_stencil_view_interface(const depth_stencil_view_interface&) = default;
			depth_stencil_view_interface(depth_stencil_view_interface&&) = default;
			depth_stencil_view_interface& operator=(const depth_stencil_view_interface&) = default;
			depth_stencil_view_interface& operator=(depth_stencil_view_interface&&) = default;
		};

		template<typename T> struct shader_resource_view : shader_resource_view_interface {
			//static_assert(Tmp::is_one_of<T, tex1, tex1_array, tex2, tex2_array, tex_cube, tex_cube_array, tex2_ms, tex2_ms_array>::value, "only receive tex1 and tex2");
		};
		template<typename T> struct render_target_view : render_target_view_interface {
			//static_assert(Tmp::is_one_of<T, tex1, tex1_array, tex2, tex2_array, tex_cube, tex_cube_array, tex2_ms, tex2_ms_array>::value, "only receive tex1 and tex2");
		};

		template<typename T> struct depth_stencil_view : depth_stencil_view_interface {
			//static_assert(Tmp::is_one_of<T, tex1, tex1_array, tex2, tex2_array, tex_cube, tex_cube_array, tex2_ms, tex2_ms_array>::value, "only receive tex1 and tex2");
		};

		template<typename T> struct unordered_access_view : unordered_access_view_interface {
			//static_assert(Tmp::is_one_of<T, tex1, tex1_array, tex2, tex2_array, tex_cube, tex_cube_array, tex2_ms, tex2_ms_array, tex3, buffer_structured>::value, "only receive tex1, tex2, tex3, and structured buffer");
		};

		struct tex1_source { 
			void* data_ptr; 
			operator D3D11_SUBRESOURCE_DATA () const { return { data_ptr, 0, 0}; }
			tex1_source(const tex1_source&) = default;
			tex1_source(tex1_source&&) = default;
			tex1_source(void* data) : data_ptr(data) {}
		};

		struct tex2_source { 
			void* data_ptr; 
			uint32_t line_size;
			operator D3D11_SUBRESOURCE_DATA () const { return { data_ptr, line_size, 0 }; }
			tex2_source(const tex2_source&) = default;
			tex2_source(tex2_source&&) = default;
			tex2_source(void* data, uint32_t line) : data_ptr(data), line_size(line) {}
		};

		struct tex3_source {
			void* data_ptr;
			uint32_t line_size;
			uint32_t surface_size;
			operator D3D11_SUBRESOURCE_DATA () const { return { data_ptr, line_size, surface_size }; }
			tex3_source(const tex3_source&) = default;
			tex3_source(tex3_source&&) = default;
			tex3_source(void* data, uint32_t line, uint32_t surface) : data_ptr(data), line_size(line), surface_size(surface){}
		};

		enum class DST_format
		{
			D16,
			D24_UI8,
			F32,
			F32_UI8,
			UNKNOW
		};

		DXGI_FORMAT translate_DST_format(DST_format DSF);


		namespace Implement
		{
			struct buffer_interface {
				Win32::com_ptr<ID3D11Buffer> ptr;
				operator bool() const { return ptr; }
			protected:
				std::optional<HRESULT> create_implement(creator& c, uint32_t width, D3D11_USAGE DU, uint32_t BIND, uint32_t misc_flag, uint32_t struct_byte, const void* data) noexcept;
			};

			struct tex1_interface {
				Win32::com_ptr<ID3D11Texture1D> ptr;
				operator bool() const { return ptr; }
				uint32_t size() const;
				using size_type = uint32_t;
				using source_type = tex1_source;
			protected:
				std::optional<HRESULT> create_implement(creator& c, DXGI_FORMAT DF, uint32_t size, uint32_t miplevel, uint32_t count, D3D11_USAGE DU, uint32_t BIND, uint32_t misc, const tex1_source* source) noexcept;
			};

			struct tex2_interface {
				Win32::com_ptr<ID3D11Texture2D> ptr;
				operator bool() const { return ptr; }
				PO::Dx::uint32_t2 size() const;
				PO::Dx::float2 size_f() const { auto s = size(); return float2{ static_cast<float>(s.x), static_cast<float>(s.y) }; }
				using size_type = uint32_t2;
				using source_type = tex2_source;
			protected:
				std::optional<HRESULT> create_implement(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel, uint32_t count, uint32_t sample_num, uint32_t sample_quality, D3D11_USAGE usage, uint32_t bind, uint32_t mis, const tex2_source* source) noexcept;
			};

			struct tex3_interface {
				Win32::com_ptr<ID3D11Texture3D> ptr;
				operator bool() const { return ptr; }
				PO::Dx::uint32_t3 size() const;
				using size_type = uint32_t3;
				using source_type = tex3_source;
			protected:
				std::optional<HRESULT> create_implement(creator& c, DXGI_FORMAT DF, uint32_t3 size, uint32_t miplevel, D3D11_USAGE usage, uint32_t bind, uint32_t mis, const tex3_source* source) noexcept;
			};

			enum class AvalibleViewType
			{
				NONE,
				SR,
				RT,
				DS,
				UA
			};
		}

		class buffer_structured : Implement::buffer_interface
		{
			using ViewType = Implement::AvalibleViewType;
			ViewType av = ViewType::NONE;
		public:
			bool create(creator& c, uint32_t size, uint32_t struct_byte, const void* data = nullptr, bool write_able = false) {
				bool result = !Implement::buffer_interface::create_implement(c, size, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, struct_byte, data).has_value();
				av = ViewType::SR;
				return result;
			}
			template<typename T, typename K> bool create(creator& c, const std::vector<T, K>& data, bool write_able = false) {
				return create(c, static_cast<uint32_t>(data.size() * sizeof(T)), sizeof(T), data.data(), write_able);
			}
			bool create_unordered_access(creator& c, uint32_t size, uint32_t struct_byte, const void* data) {
				bool result = !Implement::buffer_interface::create_implement(c, size, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, struct_byte, data).has_value();
				av = ViewType::RT;
				return result;
			}
			shader_resource_view<buffer_structured> cast_shader_resource_view(creator& c, std::optional<uint32_t2> range = {}) const;
			unordered_access_view<buffer_structured> cast_unordered_access_view(creator& c, std::optional<uint32_t2> elemnt_start_and_count = {}, std::optional<uint32_t> offset = {}, bool is_append_or_consume = false) const;
		};

		struct buffer_vertex : Implement::buffer_interface{
			uint32_t stride;
			bool create_vertex(creator& c, uint32_t size, uint32_t struct_byte, const void* data, bool write_able = false) {
				bool result = !Implement::buffer_interface::create_implement(c, size, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_VERTEX_BUFFER, 0, 0, data).has_value();
				if (result) stride = struct_byte;
				return result;
			}
			template<typename T, size_t count> bool create_vertex(creator& c, const std::array<T, count>& arr, bool write_able = false) {
				return create_vertex(c, sizeof(T) * count, sizeof(T), arr.data(), write_able);
			}
			std::tuple<buffer_vertex, uint32_t> operator[](uint32_t i) && {return std::tuple<buffer_vertex, uint32_t>{std::move(*this), i}; }
			std::tuple<const buffer_vertex&, uint32_t> operator[](uint32_t i) const& { return std::tuple<const buffer_vertex&, uint32_t>{*this, i}; }
		};

		struct buffer_index : Implement::buffer_interface {
			DXGI_FORMAT DF = DXGI_FORMAT_UNKNOWN;
			bool create_index(creator& c, uint32_t count, const uint32_t* data, bool write_able = false) {
				bool result = !Implement::buffer_interface::create_implement(c, count * sizeof(uint32_t), write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_INDEX_BUFFER, 0, 0, data).has_value();
				if (result) DF = DXGI::data_format<uint32_t>::format;
				return result;
			}
			bool create_index(creator& c, uint32_t count, const uint16_t* data, bool write_able = false) {
				bool result = !Implement::buffer_interface::create_implement(c, count * sizeof(uint16_t), write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_INDEX_BUFFER, 0, 0, data).has_value();
				if (result) DF = DXGI::data_format<uint16_t>::format;
				return result;
			}
			template<typename T, size_t count> bool create_index(creator& c, const std::array<T, count>& arr, bool write_able = false) {
				return create_index(c, count, arr.data(), write_able);
			}
		};

		struct buffer_constant : Implement::buffer_interface{
			std::tuple<buffer_constant, uint32_t> operator[](uint32_t i) && {return std::tuple<buffer_constant, uint32_t>{std::move(*this), i}; }
			std::tuple<const buffer_constant&, uint32_t> operator[](uint32_t i) const& { return std::tuple<const buffer_constant&, uint32_t>{*this, i}; }
			bool create_raw(creator& c, uint32_t width, const void* data = nullptr);
			template<typename T> bool create_pod(creator& c, const T& pod)
			{
				return create_raw(c, sizeof(T), &pod);
			}
		};

		class tex1 : Implement::tex1_interface
		{
			using ViewType = Implement::AvalibleViewType;
			ViewType av = ViewType::NONE;
		public:
			bool create(creator& c, DXGI_FORMAT DF, uint32_t size, uint32_t miplevel = 1, bool write_able = false, const tex1_source* source = nullptr) {
				bool result = !Implement::tex1_interface::create_implement(c, DF, size, miplevel, 1, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, source).has_value();
				if (result) av = ViewType::SR;
				return result;
			}
			bool create_render_target(creator& c, DXGI_FORMAT DF, uint32_t size, uint32_t miplevel = 1, const tex1_source* source = nullptr) {
				bool result = !Implement::tex1_interface::create_implement(c, DF, size, miplevel, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, source).has_value();
				if (result) av = ViewType::RT;
				return result;
			}
			bool create_depth_stencil(creator& c, DST_format DF, uint32_t size, uint32_t miplevel = 1, const tex1_source* source = nullptr) {
				bool result = !Implement::tex1_interface::create_implement(c, translate_DST_format(DF), size, miplevel, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL, 0, source).has_value();
				if (result) av = ViewType::DS;
				return result;
			}
			bool create_unordered_access(creator& c, DXGI_FORMAT DF, uint32_t size, uint32_t miplevel = 1, const tex1_source* source = nullptr) {
				bool result = !Implement::tex1_interface::create_implement(c, DF, size, miplevel, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, source).has_value();
				if (result) av = ViewType::UA;
				return result;
			}
			shader_resource_view<tex1> cast_shader_resource_view(creator& c, std::optional<uint32_t2> miplevel_start_range = {}) const { return cast_shader_resource_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel_start_range); }
			shader_resource_view<tex1> cast_shader_resource_view_as_format(creator& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_start_range = {}) const;
			depth_stencil_view<tex1> cast_depth_stencil_view(creator& c, uint32_t miplevel = 0) const;
			render_target_view<tex1> cast_render_target_view(creator& c, uint32_t miplevel = 0) const { return cast_render_target_view_as_format(c,DXGI_FORMAT_UNKNOWN, miplevel); }
			render_target_view<tex1> cast_render_target_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0) const;
			unordered_access_view<tex1> cast_unordered_access_view(creator& c, uint32_t miplevel = 0) const { return cast_unordered_access_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel); }
			unordered_access_view<tex1> cast_unordered_access_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0) const;
		};

		class tex1_array : public Implement::tex1_interface
		{
			using ViewType = Implement::AvalibleViewType;
			ViewType av = ViewType::NONE;
		public:
			bool create(creator& c, DXGI_FORMAT DF, uint32_t size, uint32_t miplevel = 1, uint32_t count = 1, bool write_able = false, const tex1_source* source = nullptr) {
				bool result = !Implement::tex1_interface::create_implement(c, DF, size, miplevel, count, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, source).has_value();
				if (result) av = ViewType::SR;
				return result;
			}
			bool create_render_target(creator& c, DXGI_FORMAT DF, uint32_t size, uint32_t miplevel = 1, uint32_t count = 1, const tex1_source* source = nullptr) {
				bool result = !Implement::tex1_interface::create_implement(c, DF, size, miplevel, count, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, source).has_value();
				if (result) av = ViewType::RT;
				return result;
			}
			bool create_depth_stencil(creator& c, DST_format DF, uint32_t size, uint32_t miplevel = 1, uint32_t count = 1, const tex1_source* source = nullptr) {
				bool result = !Implement::tex1_interface::create_implement(c, translate_DST_format(DF), size, miplevel, count, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL, 0, source).has_value();
				if (result) av = ViewType::DS;
				return result;
			}
			bool create_unordered_access(creator& c, DXGI_FORMAT DF, uint32_t size, uint32_t miplevel = 1, uint32_t count = 1, const tex1_source* source = nullptr) {
				bool result = !Implement::tex1_interface::create_implement(c, DF, size, miplevel, count, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, source).has_value();
				if (result) av = ViewType::UA;
				return result;
			}
			shader_resource_view<tex1_array> cast_shader_resource_view(creator& c, std::optional<uint32_t2> miplevel_range = {}, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_shader_resource_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel_range, array_start_and_count); }
			shader_resource_view<tex1_array> cast_shader_resource_view_as_format(creator& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range = {}, std::optional<uint32_t2> array_start_and_count = {})const;
			depth_stencil_view<tex1_array> cast_depth_stencil_view(creator& c, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {})const;
			render_target_view<tex1_array> cast_render_target_view(creator& c, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_render_target_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel, array_start_and_count); }
			render_target_view<tex1_array> cast_render_target_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {})const;
			unordered_access_view<tex1_array> cast_unordered_access_view(creator& c, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_unordered_access_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel, array_start_and_count); }
			unordered_access_view<tex1_array> cast_unordered_access_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {})const;
		};

		class tex2 : public Implement::tex2_interface
		{
			using ViewType = Implement::AvalibleViewType;
			ViewType av = ViewType::NONE;
		public:
			bool create(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, bool write_able = false, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, 1, 1, 0, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, source).has_value();
				if (result) av = ViewType::SR;
				return result;
			}
			bool create_render_target(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, 1, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, source).has_value();
				if (result) av = ViewType::RT;
				return result;
			}
			bool create_depth_stencil(creator& c, DST_format DF, uint32_t2 size, uint32_t miplevel = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, translate_DST_format(DF), size, miplevel, 1, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL, 0, source).has_value();
				if (result) av = ViewType::DS;
				return result;
			}
			bool create_unordered_access(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, 1, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, source).has_value();
				if (result) av = ViewType::UA;
				return result;
			}
			shader_resource_view<tex2> cast_shader_resource_view(creator& c, std::optional<uint32_t2> miplevel_start_range = {}) const { return cast_shader_resource_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel_start_range); }
			shader_resource_view<tex2> cast_shader_resource_view_as_format(creator& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_start_range = {}) const;
			depth_stencil_view<tex2> cast_depth_stencil_view(creator& c, uint32_t miplevel = 0)const;
			render_target_view<tex2> cast_render_target_view(creator& c, uint32_t miplevel = 0) const { return cast_render_target_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel); }
			render_target_view<tex2> cast_render_target_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0) const;
			unordered_access_view<tex2> cast_unordered_access_view(creator& c, uint32_t miplevel = 0) const { return cast_unordered_access_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel); }
			unordered_access_view<tex2> cast_unordered_access_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0)const;
		};

		class tex2_array : public Implement::tex2_interface
		{
		protected:
			using ViewType = Implement::AvalibleViewType;
			ViewType av = ViewType::NONE;
		public:
			bool create(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, uint32_t count = 1, bool write_able = false, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, count, 1, 0, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, source).has_value();
				if (result) av = ViewType::SR;
				return result;
			}
			bool create_render_target(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, uint32_t count = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, count, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, source).has_value();
				if (result) av = ViewType::RT;
				return result;
			}
			bool create_depth_stencil(creator& c, DST_format DF, uint32_t2 size, uint32_t miplevel = 1, uint32_t count = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, translate_DST_format(DF), size, miplevel, count, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL, 0, source).has_value();
				if (result) av = ViewType::DS;
				return result;
			}
			bool create_unordered_access(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, uint32_t count = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, count, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, source).has_value();
				if (result) av = ViewType::UA;
				return result;
			}
			shader_resource_view<tex2_array> cast_shader_resource_view(creator& c, std::optional<uint32_t2> miplevel_range = {}, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_shader_resource_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel_range, array_start_and_count); }
			shader_resource_view<tex2_array> cast_shader_resource_view_as_format(creator& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range = {}, std::optional<uint32_t2> array_start_and_count = {})const;
			depth_stencil_view<tex2_array> cast_depth_stencil_view(creator& c, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {})const;
			render_target_view<tex2_array> cast_render_target_view(creator& c, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_render_target_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel, array_start_and_count); }
			render_target_view<tex2_array> cast_render_target_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {})const;
			unordered_access_view<tex2_array> cast_unordered_access_view(creator& c, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_unordered_access_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel, array_start_and_count); }
			unordered_access_view<tex2_array> cast_unordered_access_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {})const;
		};

		class tex2ms : public Implement::tex2_interface
		{
			using ViewType = Implement::AvalibleViewType;
			ViewType av = ViewType::NONE;
		public:
			bool create(creator& c, DXGI_FORMAT DF, uint32_t2 size, std::optional<uint32_t2> sample_count_quality = {}, bool write_able = false, const tex2_source* source = nullptr) {
				uint32_t2 sample = sample_count_quality.value_or(uint32_t2{ 1, 0 });
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, 1, 1, sample.x, sample.y, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, source).has_value();
				if (result) av = ViewType::SR;
				return result;
			}
			bool create_render_target(creator& c, DXGI_FORMAT DF, uint32_t2 size, std::optional<uint32_t2> sample_count_quality = {}, const tex2_source* source = nullptr) {
				uint32_t2 sample = sample_count_quality.value_or(uint32_t2{ 1, 0 });
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, 1, 1, sample.x, sample.y, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, source).has_value();
				if (result) av = ViewType::RT;
				return result;
			}
			bool create_depth_stencil(creator& c, DST_format DF, uint32_t2 size, std::optional<uint32_t2> sample_count_quality = {}, const tex2_source* source = nullptr) {
				uint32_t2 sample = sample_count_quality.value_or(uint32_t2{ 1, 0 });
				bool result = !Implement::tex2_interface::create_implement(c, translate_DST_format(DF), size, 1, 1, sample.x, sample.y, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL, 0, source).has_value();
				if (result) av = ViewType::DS;
				return result;
			}
			shader_resource_view<tex2ms> cast_shader_resource_view(creator& c) const { return cast_shader_resource_view_as_format(c, DXGI_FORMAT_UNKNOWN); }
			shader_resource_view<tex2ms> cast_shader_resource_view_as_format(creator& c, DXGI_FORMAT DF)const;
			depth_stencil_view<tex2ms> cast_depth_stencil_view(creator& c)const;
			render_target_view<tex2ms> cast_render_target_view(creator& c)const { return cast_render_target_view_as_format(c, DXGI_FORMAT_UNKNOWN); }
			render_target_view<tex2ms> cast_render_target_view_as_format(creator& c, DXGI_FORMAT DF)const;
		};

		class tex2ms_array : public Implement::tex2_interface
		{
			using ViewType = Implement::AvalibleViewType;
			ViewType av = ViewType::NONE;
		public:
			bool create(creator& c, DXGI_FORMAT DF, uint32_t2 size, std::optional<uint32_t2> sample_count_quality = {}, uint32_t count = 1, bool write_able = false, const tex2_source* source = nullptr) {
				uint32_t2 sample = sample_count_quality.value_or(uint32_t2{ 1, 0 });
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, 1, count, sample.x, sample.y, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, source).has_value();
				if (result) av = ViewType::SR;
				return result;
			}
			bool create_render_target(creator& c, DXGI_FORMAT DF, uint32_t2 size, std::optional<uint32_t2> sample_count_quality = {}, uint32_t count = 1, const tex2_source* source = nullptr) {
				uint32_t2 sample = sample_count_quality.value_or(uint32_t2{ 1, 0 });
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, 1, count, sample.x, sample.y, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, source).has_value();
				if (result) av = ViewType::RT;
				return result;
			}
			bool create_depth_stencil(creator& c, DST_format DF, uint32_t2 size, std::optional<uint32_t2> sample_count_quality = {}, uint32_t count = 1, const tex2_source* source = nullptr) {
				uint32_t2 sample = sample_count_quality.value_or(uint32_t2{ 1, 0 });
				bool result = !Implement::tex2_interface::create_implement(c, translate_DST_format(DF), size, 1, count, sample.x, sample.y, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL, 0, source).has_value();
				if (result) av = ViewType::DS;
				return result;
			}
			shader_resource_view<tex2ms_array> cast_shader_resource_view(creator& c, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_shader_resource_view_as_format(c, DXGI_FORMAT_UNKNOWN, array_start_and_count); }
			shader_resource_view<tex2ms_array> cast_shader_resource_view_as_format(creator& c, DXGI_FORMAT DF, std::optional<uint32_t2> array_start_and_count = {})const;
			depth_stencil_view<tex2ms_array> cast_depth_stencil_view(creator& c, std::optional<uint32_t2> array_start_and_count = {})const;
			render_target_view<tex2ms_array> cast_render_target_view(creator& c, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_render_target_view_as_format(c, DXGI_FORMAT_UNKNOWN, array_start_and_count); }
			render_target_view<tex2ms_array> cast_render_target_view_as_format(creator& c, DXGI_FORMAT DF, std::optional<uint32_t2> array_start_and_count = {})const;
		};

		class tex_cube : public tex2_array
		{
		public:
			bool create(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, bool write_able = false, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, 6, 1, 0, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, D3D11_RESOURCE_MISC_TEXTURECUBE, source).has_value();
				if (result) av = ViewType::SR;
				return result;
			}
			bool create_render_target(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, 6, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, D3D11_RESOURCE_MISC_TEXTURECUBE, source).has_value();
				if (result) av = ViewType::RT;
				return result;
			}
			bool create_depth_stencil(creator& c, DST_format DF, uint32_t2 size, uint32_t miplevel = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, translate_DST_format(DF), size, miplevel, 6, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL, D3D11_RESOURCE_MISC_TEXTURECUBE, source).has_value();
				if (result) av = ViewType::DS;
				return result;
			}
			bool create_unordered_access(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, 6, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, D3D11_RESOURCE_MISC_TEXTURECUBE, source).has_value();
				if (result) av = ViewType::UA;
				return result;
			}
			shader_resource_view<tex_cube> cast_shader_resource_view(creator& c, std::optional<uint32_t2> miplevel_range = {}) const { return cast_shader_resource_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel_range); }
			shader_resource_view<tex_cube> cast_shader_resource_view_as_format(creator& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range = {})const;
			using tex2_array::cast_depth_stencil_view;
			using tex2_array::cast_render_target_view;
			using tex2_array::cast_unordered_access_view;
			using tex2_array::cast_unordered_access_view_as_format;
		};

		class tex_cube_array : public tex2_array
		{
		public:
			bool create(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, uint32_t count = 1, bool write_able = false, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, 6 * count, 1, 0, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, D3D11_RESOURCE_MISC_TEXTURECUBE, source).has_value();
				if (result) av = ViewType::SR;
				return result;
			}
			bool create_render_target(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, uint32_t count = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, 6 * count, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, D3D11_RESOURCE_MISC_TEXTURECUBE, source).has_value();
				if (result) av = ViewType::RT;
				return result;
			}
			bool create_depth_stencil(creator& c, DST_format DF, uint32_t2 size, uint32_t miplevel = 1, uint32_t count = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, translate_DST_format(DF), size, miplevel, 6 * count, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL, D3D11_RESOURCE_MISC_TEXTURECUBE, source).has_value();
				if (result) av = ViewType::DS;
				return result;
			}
			bool create_unordered_access(creator& c, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel = 1, uint32_t count = 1, const tex2_source* source = nullptr) {
				bool result = !Implement::tex2_interface::create_implement(c, DF, size, miplevel, 6 * count, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, D3D11_RESOURCE_MISC_TEXTURECUBE, source).has_value();
				if (result) av = ViewType::UA;
				return result;
			}
			shader_resource_view<tex_cube_array> cast_shader_resource_view(creator& c, std::optional<uint32_t2> miplevel_range = {}, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_shader_resource_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel_range, array_start_and_count); }
			shader_resource_view<tex_cube_array> cast_shader_resource_view_as_format(creator& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range = {}, std::optional<uint32_t2> array_start_and_count = {})const;
			using tex2_array::cast_depth_stencil_view;
			using tex2_array::cast_render_target_view;
			using tex2_array::cast_unordered_access_view;
			using tex2_array::cast_unordered_access_view_as_format;
		};

		class tex3 : public Implement::tex3_interface
		{
		public:
			bool create(creator& c, DXGI_FORMAT DF, uint32_t3 size, uint32_t miplevel = 1, bool write_able = false, const tex3_source* source = nullptr) {
				bool result = !Implement::tex3_interface::create_implement(c, DF, size, miplevel, write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, source).has_value();
				return result;
			}
			bool create_render_target(creator& c, DXGI_FORMAT DF, uint32_t3 size, uint32_t miplevel = 1, const tex3_source* source = nullptr) {
				bool result = !Implement::tex3_interface::create_implement(c, DF, size, miplevel, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, source).has_value();
				return result;
			}
			bool create_unordered_access(creator& c, DXGI_FORMAT DF, uint32_t3 size, uint32_t miplevel = 1, const tex3_source* source = nullptr) {
				bool result = !Implement::tex3_interface::create_implement(c, DF, size, miplevel, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, source).has_value();
				return result;
			}
			shader_resource_view<tex3> cast_shader_resource_view(creator& c, std::optional<uint32_t2> miplevel_range = {}) const { return cast_shader_resource_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel_range); }
			shader_resource_view<tex3> cast_shader_resource_view_as_format(creator& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range = {})const;
			render_target_view<tex3> cast_render_target_view(creator& c, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_render_target_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel, array_start_and_count); }
			render_target_view<tex3> cast_render_target_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {})const;
			unordered_access_view<tex3> cast_unordered_access_view(creator& c, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {}) const { return cast_unordered_access_view_as_format(c, DXGI_FORMAT_UNKNOWN, miplevel, array_start_and_count); }
			unordered_access_view<tex3> cast_unordered_access_view_as_format(creator& c, DXGI_FORMAT DF, uint32_t miplevel = 0, std::optional<uint32_t2> array_start_and_count = {})const;
		};

		struct sample_state { 
			using description = D3D11_SAMPLER_DESC;
			static description default_description;
			operator bool() const { return ptr; }
			Win32::com_ptr<ID3D11SamplerState> ptr;
			std::tuple<sample_state, uint32_t> operator[](uint32_t i) && {return std::tuple<sample_state, uint32_t>{std::move(*this), i}; }
			std::tuple<const sample_state&, uint32_t> operator[](uint32_t i) const& { return std::tuple<const sample_state&, uint32_t>{std::move(*this), i}; }
			void create(creator& c, const description& = default_description);
		};

		struct viewport
		{
			D3D11_VIEWPORT view;
			viewport(float2 width_height = float2(0.0, 0.0), float2 top_left = float2(0.0, 0.0), float2 min_max_depth = { 0.0, 1.0 }) : view{ top_left.x, top_left.y, width_height.x, width_height.y, min_max_depth.x, min_max_depth.y } {}
			viewport(const viewport&) = default;
			viewport& operator=(const viewport&) = default;
		};

		struct scissor
		{
			D3D11_RECT rect;
			scissor(uint32_t2 left_top, uint32_t2 right_buttom) : rect{ static_cast<LONG>(left_top.x), static_cast<LONG>(left_top.y), static_cast<LONG>(right_buttom.x), static_cast<LONG>(right_buttom.y) } {}
			scissor(const scissor&) = default;
			scissor& operator=(const scissor&) = default;
		};

		struct raterizer_state {
			using description = D3D11_RASTERIZER_DESC;
			static description default_description;
			Win32::com_ptr<ID3D11RasterizerState> ptr;
			operator bool() const { return ptr; }
			void create(creator& c, const description& = default_description);
		};

		struct blend_state {
			using description = D3D11_BLEND_DESC;
			static description default_description;

			Win32::com_ptr<ID3D11BlendState> ptr;
			operator bool() const { return ptr; }
			std::array<float, 4> bind_factor = {1.0f, 1.0f, 1.0f, 1.0f};
			uint32_t sample_mask = 0xffffffff;
			void create(creator& c, const description& = default_description);
		};

		struct depth_stencil_state {
			using description = D3D11_DEPTH_STENCIL_DESC;
			static description default_description;

			Win32::com_ptr<ID3D11DepthStencilState> ptr;
			operator bool() const { return ptr; }
			uint32_t stencil_ref = 0;
			void create(creator& c, const description& = default_description);
		};

		struct shader_vertex
		{
			std::shared_ptr<PO::Dx::shader_binary> code;
			Win32::com_ptr<ID3D11VertexShader> ptr;
			operator bool() const { return ptr; }
			void create(creator& c, std::shared_ptr<PO::Dx::shader_binary> code);
		};

		struct shader_pixel
		{
			Win32::com_ptr<ID3D11PixelShader> ptr;
			operator bool() const { return ptr; }
			void create(creator& c, const PO::Dx::shader_binary& code);
		};

		struct shader_compute
		{
			Win32::com_ptr<ID3D11ComputeShader> ptr;
			operator bool() const { return ptr; }
			void create(creator& c, const PO::Dx::shader_binary& code);
		};

		template<typename syntax_t, size_t i, typename store_type, size_t ic = 0, bool force_instance = false>
		struct syntax : DXGI::data_format<store_type>
		{
			static constexpr uint32_t align = alignof(store_type);
			static constexpr uint32_t size = sizeof(store_type);
			static constexpr uint32_t index = i;
			static constexpr D3D11_INPUT_CLASSIFICATION buffer_type = ((!force_instance && ic == 0) ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA);
			static constexpr uint32_t instance_used = ic;
			static const char* name() { return syntax_t{}(); }
		};

		template<typename T, typename ...AT> struct layout_type;

		namespace Implement
		{
			template<size_t d, typename T> struct layout_element
			{
				using type = T;
				void operator()(D3D11_INPUT_ELEMENT_DESC* DIED, uint32_t solt) const
				{
					*DIED = D3D11_INPUT_ELEMENT_DESC{
						T::name(), T::index, T::format, static_cast<UINT>(solt), static_cast<UINT>(d),
						T::buffer_type,
						static_cast<UINT>(T::instance_used)
					};
				}
			};

			template<typename its, typename ...oth> struct final_layout
			{
				static_assert(!Tmp::is_repeat<typename its::type, typename oth::type...>::value, "layout can not contain same element");

				void operator()(D3D11_INPUT_ELEMENT_DESC* DIED, uint32_t solt) const
				{
					its{}(DIED, solt);
					final_layout<oth...>{}(DIED + 1, solt);
				}
			};

			template<typename its> struct final_layout<its>
			{
				void operator()(D3D11_INPUT_ELEMENT_DESC* DIED, uint32_t solt) const
				{
					its{}(DIED, solt);
				}
			};

			template<typename input, size_t last, typename ...AT> struct make_layout_execute;
			template<typename ...input, size_t last, typename its, typename ...AT> struct make_layout_execute<std::tuple<input...>, last, its, AT...>
			{
				using type = typename make_layout_execute<
					std::tuple<input..., layout_element<last, its>>,
					((last % its::align) == 0 ? (last + its::size) : (last + its::align - (last %its::align))),
					//((( last + its::size ) % 4) == 0 ? (last + its::size) : ((last /4 +1 )*4)),
					AT...
				>::type;
			};

			/*
			template<typename ...input, size_t last, size_t count, typename ...its, typename ...AT> struct make_layout_execute<std::tuple<input...>, last, count, layout_type<its...>, AT...>
			{
				using type = typename make_layout_execute <
					std::tuple<input...>, last, count, its..., AT...
				>::type;
			};
			*/

			template<typename ...input, size_t last> struct make_layout_execute<std::tuple<input...>, last>
			{
				using type = final_layout<input...>;
			};

		}

		template<typename T, typename ...AT> struct buffer_layout
		{
			using type = typename Implement::make_layout_execute<std::tuple<>, 0, T, AT...>::type;
			static constexpr size_t size = sizeof...(AT)+1;
			void operator()(D3D11_INPUT_ELEMENT_DESC* DIED, uint32_t solt) const
			{
				type{}(DIED, solt);
			}
		};

		namespace Implement
		{
			template<typename T, typename ...AT> struct layout_type_implement
			{
				void operator()(D3D11_INPUT_ELEMENT_DESC* DIED, uint32_t solt)
				{
					T{}(DIED, solt);
					layout_type_implement<AT...>{}(DIED + T::size, solt + 1);
				}
			};
			template<typename T> struct layout_type_implement<T>
			{
				void operator()(D3D11_INPUT_ELEMENT_DESC* DIED, uint32_t solt)
				{
					T{}(DIED, solt);
				}
			};
			template<size_t count> struct layout_carry
			{
				std::array<D3D11_INPUT_ELEMENT_DESC, count> carry;
				template<typename T> layout_carry(T&& t) { t(carry.data(), 0); }
			};
		}

		struct layout_view
		{
			const D3D11_INPUT_ELEMENT_DESC* ptr = nullptr;
			uint32_t size = 0;
		};

		template<typename T, typename ...AT> struct layout_type
		{
			static constexpr size_t size = Tmp::value_add<size_t, T::size, AT::size...>::value;
			operator layout_view() const {
				static Implement::layout_carry<size> static_array(Implement::layout_type_implement<T, AT...>{});
				return { static_array.carry.data(), size };
			}
		};



		struct input_layout
		{
			Win32::com_ptr<ID3D11InputLayout> ptr;
			operator bool() const { return ptr; }
			void create(creator&, const layout_view& view, const shader_vertex& sv);
			template<typename T, typename ...AT> void create(creator& c, const layout_type<T, AT...>& t, const shader_vertex& sv)
			{
				create(c, t.cast_layout_view(), sv);
			}
		};

		using primitive_topology = D3D11_PRIMITIVE_TOPOLOGY;

		struct output_merge_stage
		{

			uint32_t avalible_render_target_size;
			std::array<ID3D11RenderTargetView*, 8> target;
			depth_stencil_view_interface depth;



			output_merge_stage& set(const render_target_view_interface& tvi);
			output_merge_stage& set(depth_stencil_view_interface dsv) { depth = std::move(dsv); return *this; }
			output_merge_stage& operator<< (const render_target_view_interface& dsv) { return set(dsv); }
			output_merge_stage& operator<< (depth_stencil_view_interface dsv) { return set(std::move(dsv)); }
			output_merge_stage::~output_merge_stage();
			void clear();
			void clear_render_target();
			output_merge_stage();
			output_merge_stage(const output_merge_stage& oms);
			output_merge_stage(output_merge_stage&& oms);
			output_merge_stage& operator=(const output_merge_stage&);
			output_merge_stage& operator=(output_merge_stage&&);
		};

		namespace Implement
		{

			struct shader_context_t
			{
				uint32_t srv_count = 0;
				uint32_t cb_count = 0;
				uint32_t sample_count = 0;

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp,
					void (__stdcall ID3D11DeviceContext::* cb_f)(uint32_t, uint32_t, ID3D11Buffer* const *),
					void (__stdcall ID3D11DeviceContext::* srv_f)(uint32_t, uint32_t, ID3D11ShaderResourceView* const *),
					void (__stdcall ID3D11DeviceContext::* sample_f)(uint32_t, uint32_t, ID3D11SamplerState* const *)
				);

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const buffer_constant& id, uint32_t solt,
					void(__stdcall ID3D11DeviceContext::* cb_f)(uint32_t, uint32_t, ID3D11Buffer* const *)
				);

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const shader_resource_view_interface& id, uint32_t solt,
					void(__stdcall ID3D11DeviceContext::* srv_f)(uint32_t, uint32_t, ID3D11ShaderResourceView* const *)
				);

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const sample_state& id, uint32_t solt,
					void(__stdcall ID3D11DeviceContext::* sample_f)(uint32_t, uint32_t, ID3D11SamplerState* const *)
				);
			};

			struct input_assember_context_t
			{
				uint32_t vb_count = 0;
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, primitive_topology tp) { cp->IASetPrimitiveTopology(tp); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const buffer_vertex& vi, uint32_t solt);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const buffer_index& iv);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const input_layout& id) { cp->IASetInputLayout(id.ptr); }
				//void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, input_layout& id);
				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp);
			};

			struct vertex_stage_context_t : shader_context_t
			{
				//size_t max_shader_resource_view = 0;
				//size_t max_sample = 0;
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const shader_vertex& vs) { cp->VSSetShader(vs.ptr, nullptr, 0); }

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp) { unbind(cp); }

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const buffer_constant& cb, uint32_t solt) { shader_context_t::bind(cp, cb, solt, &ID3D11DeviceContext::VSSetConstantBuffers); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const shader_resource_view_interface& cb, uint32_t solt) { shader_context_t::bind(cp, cb, solt, &ID3D11DeviceContext::VSSetShaderResources); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const sample_state& cb, uint32_t solt) { shader_context_t::bind(cp, cb, solt, &ID3D11DeviceContext::VSSetSamplers); }
			};

			struct raterizer_context_t
			{
				//binding_count view_count;

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const raterizer_state& rs);
				//void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const viewports& rs);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const viewport& rs);

				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, raterizer_state& rs);
				//void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, viewports& rs);

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp);
			};

			struct pixel_stage_context_t : shader_context_t
			{
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const shader_pixel& ps) { cp->PSSetShader(ps.ptr, nullptr, 0); }

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp) { unbind(cp); }

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const buffer_constant& cb, uint32_t solt) { shader_context_t::bind(cp, cb, solt, &ID3D11DeviceContext::PSSetConstantBuffers); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const shader_resource_view_interface& cb, uint32_t solt) { shader_context_t::bind(cp, cb, solt, &ID3D11DeviceContext::PSSetShaderResources); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const sample_state& cb, uint32_t solt) { shader_context_t::bind(cp, cb, solt, &ID3D11DeviceContext::PSSetSamplers); }

			};

			struct output_merge_context_t
			{
				void clear_render_target(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, uint32_t solt, const std::array<float, 4>& color);
				void clear_render_target(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, const std::array<float, 4>& color);
				void clear_depth(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, float depth);
				void clear_stencil(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, uint8_t ref);
				void clear_depth_stencil(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, float depth, uint8_t ref);

				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& ps);

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const output_merge_stage&);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const blend_state&);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const depth_stencil_state&);
				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp) { unbind(cp); }
			};

			struct compute_stage_context_t : shader_context_t
			{
				uint32_t count = 0;
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const shader_compute& cd) { cp->CSSetShader(cd.ptr, nullptr, 0); }

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp) { unbind(cp); }

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const buffer_constant& cb, uint32_t solt) { shader_context_t::bind(cp, cb, solt, &ID3D11DeviceContext::CSSetConstantBuffers); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const shader_resource_view_interface& cb, uint32_t solt) { shader_context_t::bind(cp, cb, solt, &ID3D11DeviceContext::CSSetShaderResources); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const sample_state& cb, uint32_t solt) { shader_context_t::bind(cp, cb, solt, &ID3D11DeviceContext::CSSetSamplers); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const unordered_access_view_interface& cb, uint32_t solt);
			};
		}

		struct dispatch_call { 
			uint32_t x, y, z; 
		};

		struct vertex_call { uint32_t count, start; };
		struct index_call { uint32_t index_count, index_start, vertex_start; };
		struct vertex_instance_call { uint32_t vertex_pre_instance, instance_count, vertex_start, instance_start; };
		struct index_instance_call { uint32_t index_pre_instance, instance_count, index_start, base_vertex, instance_start; };

		struct stage_context_implement
		{
			Win32::com_ptr<ID3D11DeviceContext> ptr;
			Implement::input_assember_context_t IA;
			Implement::vertex_stage_context_t VS;
			Implement::raterizer_context_t RA;
			Implement::pixel_stage_context_t PS;
			Implement::output_merge_context_t OM;

			Implement::compute_stage_context_t CS;

			Tool::variant<dispatch_call, vertex_call, index_call, vertex_instance_call, index_instance_call> call_require;

			stage_context_implement(Win32::com_ptr<ID3D11DeviceContext> cp);
			operator bool() const { return ptr; }
			~stage_context_implement() { clear(); }
			void clear();

			/*
			void dispatch(uint32_t x, uint32_t y, uint32_t z);

			void draw_vertex(uint32_t count, uint32_t start);
			void draw_index(uint32_t index_count, uint32_t index_start, uint32_t vertex_start);
			void draw_vertex_instance(uint32_t vertex_pre_instance, uint32_t instance_count, uint32_t vertex_start, uint32_t instance_start);
			void draw_index_instance(uint32_t index_pre_instance, uint32_t instance_count, uint32_t index_start, uint32_t base_vertex, uint32_t instance_start);
			*/

			void unbind();

			stage_context_implement& bind(primitive_topology d) { IA.bind(ptr, d); return *this; }
			stage_context_implement& bind(const input_layout& d) { IA.bind(ptr, d); return *this; }
			stage_context_implement& bind(const buffer_index& d) { IA.bind(ptr, d); return *this; }
			stage_context_implement& bind(const buffer_vertex& vv, uint32_t solt) { IA.bind(ptr, vv, solt); return *this; }
			//stage_context_implement& bind(const vertex_resource& d) { VS.bind(ptr, d); return *this; }
			stage_context_implement& bind(const shader_vertex& d) { VS.bind(ptr, d); return *this; }
			//stage_context_implement& bind(const pixel_stage& d) { PS.bind(ptr, d); return *this; }
			//stage_context_implement& bind(const pixel_resource& d) { PS.bind(ptr, d); return *this; }
			stage_context_implement& bind(const shader_pixel& d) { PS.bind(ptr, d); return *this; }
			stage_context_implement& bind(const output_merge_stage& d) { OM.bind(ptr, d); return *this; }
			stage_context_implement& bind(const raterizer_state& rs) { RA.bind(ptr, rs); return *this; }
			//stage_context_implement& bind(const compute_stage& cd) { CS.bind(ptr, cd); return *this; }
			//stage_context_implement& bind(const compute_resource& cd) { CS.bind(ptr, cd); return *this; }
			stage_context_implement& bind(const shader_compute& cd) { CS.bind(ptr, cd); return *this; }
			stage_context_implement& bind(const viewport& vp) { RA.bind(ptr, vp); return *this; }
			stage_context_implement& bind(const blend_state& bs) { OM.bind(ptr, bs); return *this; }
			stage_context_implement& bind(const depth_stencil_state& dss) { OM.bind(ptr, dss); return *this; }

			stage_context_implement& bind(const dispatch_call& d) { call_require = d; return *this; }
			stage_context_implement& bind(const vertex_call& d) { call_require = d; return *this; }
			stage_context_implement& bind(const index_call& d) { call_require = d; return *this; }
			stage_context_implement& bind(const vertex_instance_call& d) { call_require = d; return *this; }
			stage_context_implement& bind(const index_instance_call& d) { call_require = d; return *this; }

			void call();

			stage_context_implement& clear_render_target(output_merge_stage& omd, uint32_t solt, const std::array<float, 4>& color) { OM.clear_render_target(ptr, omd, solt, color); return *this; }
			stage_context_implement& clear_render_target(output_merge_stage& omd, const std::array<float, 4>& color) { OM.clear_render_target(ptr, omd, color); return *this;}
			stage_context_implement& clear_depth(output_merge_stage& omd, float depth) { OM.clear_depth(ptr, omd, depth); return *this;}
			stage_context_implement& clear_stencil(output_merge_stage& omd, uint8_t ref) { OM.clear_stencil(ptr, omd, ref); return *this;}
			stage_context_implement& clear_depth_stencil(output_merge_stage& omd, float depth, uint8_t ref) { OM.clear_depth_stencil(ptr, omd, depth, ref); return *this;}

			/*
			template<typename T> bool write_buffer_constant(shader_resource& b, size_t o, T&& t)
			{
				if (b.cbuffer_array.size() <= o) return false;
				D3D11_MAPPED_SUBRESOURCE DMS;
				auto ptr_buffer = b.cbuffer_array[o];
				if (SUCCEEDED(ptr->Map(ptr_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &DMS)))
				{
					Tool::at_scope_exit ate([&, this]() { ptr->Unmap(ptr_buffer, 0); });
					return Tool::auto_adapter_unorder(t, DMS.pData, DMS.RowPitch, DMS.DepthPitch), true;
				}
				return false;
			}*/

		};

		template<typename T> struct stage_reference
		{
			Win32::com_ptr<ID3D11DeviceContext>& ptr;
			std::decay_t<T>& t;
			template<typename F>
			stage_reference operator<<(const std::tuple<F, uint32_t>& ts) { t.bind(ptr, std::get<0>(ts), std::get<1>(ts)); return *this; }
		};

		
		struct stage_context : creator 
		{
			std::shared_ptr<stage_context_implement> imp;
			operator bool() const { return static_cast<bool>(imp); }
			stage_context(std::shared_ptr<stage_context_implement> ptr, Win32::com_ptr<ID3D11Device> p) : creator(std::move(p)), imp(std::move(ptr)) { assert(imp); }
			stage_context(const stage_context& pl) : imp(pl.imp), creator(pl) { assert(imp); }

			stage_reference<Implement::input_assember_context_t> IA() { return stage_reference<decltype(imp->IA)> { imp->ptr, imp->IA}; }
			stage_reference<Implement::vertex_stage_context_t> VS() { return stage_reference<decltype(imp->VS)> { imp->ptr, imp->VS}; }
			stage_reference<Implement::pixel_stage_context_t> PS() { return stage_reference<decltype(imp->PS)> { imp->ptr, imp->PS}; }
			stage_reference<Implement::compute_stage_context_t> CS() { return stage_reference<decltype(imp->CS)> { imp->ptr, imp->CS}; }

			template<typename T> stage_context& operator<<(const T& t) { imp->bind(t); return *this; }
			stage_context& operator<<(const std::tuple<const buffer_vertex&, uint32_t>& t) { imp->bind(std::get<0>(t), std::get<1>(t)); return *this; }
			stage_context& operator<<(const std::tuple<buffer_vertex, uint32_t>& t) { imp->bind(std::get<0>(t), std::get<1>(t)); return *this; }
			void unbind() { imp->unbind(); }
			stage_context& clear_render_target(output_merge_stage& omd, uint32_t solt, const std::array<float, 4>& color) { imp->clear_render_target(omd, solt, color); return *this; }
			stage_context& clear_render_target(output_merge_stage& omd, const std::array<float, 4>& color) { imp->clear_render_target(omd, color); return *this; }
			stage_context& clear_depth(output_merge_stage& omd, float depth) { imp->clear_depth(omd, depth); return *this; }
			stage_context& clear_stencil(output_merge_stage& omd, uint8_t ref) { imp->clear_stencil(omd, ref); return *this; }
			stage_context& clear_depth_stencil(output_merge_stage& omd, float depth, uint8_t ref) { imp->clear_depth_stencil(omd, depth, ref); return *this; }

			/*
			void draw_vertex(uint32_t count, uint32_t start) { imp->draw_vertex(count, start); }
			void draw_index(uint32_t index_count, uint32_t index_start, uint32_t vertex_start) { imp->draw_index(index_count, index_start, vertex_start); }
			void draw_vertex_instance(uint32_t vertex_pre_instance, uint32_t instance_count, uint32_t vertex_start, uint32_t instance_start) { imp->draw_vertex_instance (vertex_pre_instance, instance_count, vertex_start, instance_start); }
			void draw_index_instance(uint32_t index_pre_instance, uint32_t instance_count, uint32_t index_start, uint32_t base_vertex, uint32_t instance_start) {
				imp->draw_index_instance (index_pre_instance, instance_count, index_start, base_vertex, instance_start);
			}
			void dispatch(uint32_t x, uint32_t y, uint32_t z) { imp->dispatch(x, y, z); }
			*/
			void call() { imp->call(); }
		};

		struct Dx11_frame_initializer
		{
			creator cre;
			stage_context sta;
			swap_chain_ptr swa;
			tex2 bac;
		};

	}
}