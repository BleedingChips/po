#pragma once
#include "format.h"
#include "../po/tool/intrusive_ptr.h"
//#include "../../po/tool/path_system.h"
#include "../po/tool/asset.h"
#include "context.h"
#include <atomic>
#include <variant>
#include <tuple>

namespace PO::Graphic::Implement
{
	template<typename T, typename ...OT> struct alignas(16) shader_storage_implement
	{
		static_assert(alignof(T) <= 16, "");
		static_assert(std::is_standard_layout_v<T>, "");
		alignas(16) T data;
		alignas(16) shader_storage_implement<OT...> next;
		template<typename TT, typename ...CT> shader_storage_implement(TT&& tt, CT&& ...ct) :data(std::forward<TT>(tt)), next(std::forward<CT>(ct)...) {}
		shader_storage_implement() {}
		T& get_implement(std::integral_constant<size_t, 0>) noexcept { return data; }
		template<size_t index> decltype(auto) get_implement(std::integral_constant<size_t, index>) { return next.get_implement(std::integral_constant<size_t, index - 1>{}); }
		const T& get_implement(std::integral_constant<size_t, 0>) const noexcept { return data; }
		template<size_t index> decltype(auto) get_implement(std::integral_constant<size_t, index>) const noexcept { return next.get_implement(std::integral_constant<size_t, index - 1>{}); }
	};

	template<typename T> struct alignas(16) shader_storage_implement<T>
	{
		static_assert(alignof(T) <= 16, "");
		static_assert(std::is_standard_layout_v<T>, "");
		alignas(16) T data;

		T& get_implement(std::integral_constant<size_t, 0>) noexcept { return data; }
		const T& get_implement(std::integral_constant<size_t, 0>) const noexcept { return data; }

		template<typename TT> shader_storage_implement(TT&& tt) :data(std::forward<TT>(tt)) {}
		shader_storage_implement() {}
	};
}

namespace std
{
	template<size_t i, typename this_t, typename ...tuple_t>
	decltype(auto) get(PO::Graphic::Implement::shader_storage_implement<this_t, tuple_t...>& p)
	{
		return p.get_implement(std::integral_constant<size_t, i>{});
	}
	template<size_t i, typename this_t, typename ...tuple_t>
	decltype(auto) get(const PO::Graphic::Implement::shader_storage_implement<this_t, tuple_t...>& p)
	{
		return p.get_implement(std::integral_constant<size_t, i>{});
	}
}

namespace PO::Graphic
{
	template<typename this_t, typename ...ot> class shader_storage : Implement::shader_storage_implement<std::decay_t<this_t>, std::decay_t<ot>...>
	{
		using type = Implement::shader_storage_implement<std::decay_t<this_t>, std::decay_t<ot>...>;
	public:
		using type::shader_storage_implement;
		template<typename callcable> decltype(auto) operator()(callcable&& c)
		{
			return Tool::apply(std::make_integer_sequence<size_t, sizeof...(ot) + 1>{}, static_cast<type&>((*this)), std::forward<callcable>(c));
		}
		template<typename callcable> decltype(auto) operator()(callcable&& c) const
		{
			return Tool::apply(std::make_integer_sequence<size_t, sizeof...(ot) + 1>{}, static_cast<const type&>((*this)), std::forward<callcable>(c));
		}
	};

	/*
	struct resource_id : Tool::intrusice_strong_bject_base {
		size_t id() const noexcept { return reinterpret_cast<size_t>(this); }
	};

	using resource_viewer_ptr = Tool::intrusive_weak_ptr<resource_id>;
	template<typename T> using resource_owner_ptr = Tool::intrusive_strong_ptr<T>;
	*/


}

namespace PO::Graphic
{

	

	struct texture
	{
		//operator bool() const noexcept { return m_resource; }
	private:
		//texture_resource_ptr m_resource;
	};
}

/*
namespace PO::Graphic
{

	template<typename this_t, typename ...ot> class shader_storage : Implement::shader_storage_implement<std::decay_t<this_t>, std::decay_t<ot>...>
	{
		using type = Implement::shader_storage_implement<std::decay_t<this_t>, std::decay_t<ot>...>;
	public:
		using type::shader_storage_implement;
		template<typename callcable> decltype(auto) operator()(callcable&& c)
		{
			return Tool::apply(std::make_integer_sequence<size_t, sizeof...(ot)+1>{}, static_cast<type&>((*this)), std::forward<callcable>(c));
		}
		template<typename callcable> decltype(auto) operator()(callcable&& c) const
		{
			return Tool::apply(std::make_integer_sequence<size_t, sizeof...(ot)+1>{}, static_cast<const type&>((*this)), std::forward<callcable>(c));
		}
	};


	// buffer setting *********************************************
	enum class BufUsage
	{
		CON,
		CON_SWAP,
		CON_OUTPUT
	};

	namespace Implement
	{
		struct alignas(16) buffer_const_description : resource_id
		{
			BufUsage usage() const noexcept { return m_usage; }
			uint64_t vision() const noexcept { return m_vision; }
			uint64_t size() const noexcept { return m_size; }
			std::byte* shift_buffer() noexcept;
			const std::byte* shift_buffer() const noexcept;
			void update() noexcept { m_vision += 1; }
			bool need_update(uint64_t& target_vision) const noexcept { if (target_vision != m_vision) return (target_vision = m_vision, true); return false; }

			static resource_owner_ptr<buffer_const_description> create(BufUsage usage, size_t size);

		private:

			BufUsage m_usage = BufUsage::CON;
			size_t m_size;
			uint64_t m_vision = 1;
		};
	}

	struct resource_solt;

	struct buffer
	{
		friend struct resource_solt;
		template<typename callable>
		size_t create(BufUsage usage, callable&& c)
		{
			using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<callable>>::type>;
			using parameter = typename funtype::template out<shader_storage>;
			reallocate_buffer(usage, sizeof(parameter));
			assert(m_buffer);
			(*reinterpret_cast<parameter*>(m_buffer->shift_buffer()))(std::forward<callable>(c));
			return 0;
		}

		template<typename callable> bool update(callable&& ca)
		{
			using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<callable>>::type>;
			using parameter = typename funtype::template out<shader_storage>;
			if (m_buffer && m_buffer->m_buffer_size >= sizeof(parameter))
			{
				(*reinterpret_cast<parameter*>(m_buffer->shift_buffer()))(std::forward<callable>(ca));
				m_buffer->m_vision += 1;
				return true;
			}
			return false;
		}

		size_t reallocate_buffer(BufUsage usage, size_t buffer);

		//void clear();

		template<typename callable> bool scan(callable&& ca) const noexcept
		{
			using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<callable>>::type>;
			using parameter = typename funtype::template out<shader_storage>;
			if (m_buffer && m_buffer->m_buffer_size >= sizeof(parameter))
			{
				(*reinterpret_cast<const parameter*>(m_buffer->shift_buffer()))(std::forward<callable>(ca));
				return true;
			}
			return false;
		}

		buffer() = default;
		buffer(buffer&& b) = default;
		buffer& operator= (buffer&& da) = default;
		operator bool() const noexcept { return m_buffer; }

	public:

		resource_owner_ptr<Implement::buffer_const_description> m_buffer;
	};

	// texture *************************************************
	enum class TexUsage
	{
		UNCHANGE,
		SWAP,
		RENDER_TARGET,
		DEPTH,
		OUTPUT
	};

	enum class SampleState
	{
		LINE,
	};

	struct tex_size
	{
		uint16_t x, y, z;
		
		bool is_1d() const noexcept { return x != 0 && y == 0 && z == 0; }
		bool is_2d() const noexcept { return x != 0 && y != 0 && z == 0; }
		bool is_3d() const noexcept { return x != 0 && y != 0 && z != 0; }

		tex_size(uint16_t x_size = 0, uint16_t y_size = 0, uint16_t z_size = 0) : x(x_size), y(y_size), z(z_size) {}
		tex_size(const tex_size&) = default;
		tex_size& operator= (const tex_size&) = default;
		size_t space() const noexcept;
	};

	struct tex_resource
	{
		FormatPixel format;
		tex_size size;
		uint16_t mipmap = 0;
	};

	struct asset_texture : Tool::asset_interface
	{
		FormatPixel format() const noexcept { return m_format; }
		size_t space() const noexcept { return calculate_pixel_size(m_format) * m_size.space(); }
		static asset_texture* allocate_asset_texture(FormatPixel format, tex_size texture_size, uint32_t mipmap = 1);
	private:
		asset_texture(FormatPixel format, tex_size size, uint64_t mipmap);
		asset_texture(const asset_texture&) = delete;
		FormatPixel m_format;
		tex_size m_size;
		uint64_t m_mipmap;
	};

	namespace Implement
	{
		struct dynamic_texture
		{
			TexUsage usage;
			FormatPixel format;
			tex_size size;
		};




		struct sampler_state
		{
			SampleState State;
		};

		struct tex_raw_resource
		{
			FormatPixel format;
			tex_size size;
			uint64_t target_mipmap;
			const bool follower_ptr;
		};

		struct tex_description : Tool::atomic_reference_count
		{
			std::atomic_bool avalible;
			const TexUsage usage;
			const std::variant<Tool::asset::id, tex_raw_resource> resource;
			tex_description(TexUsage u, const tex_raw_resource& trr) : usage(u), resource({ trr }) {}
			tex_description(TexUsage u, Tool::asset::id id) : usage(u), resource({ id }) {}
		};

	}

	struct texture
	{

		struct deleter { void operator()(Implement::tex_description*) noexcept; };
		using description_ptr = Tool::intrusive_ptr<Implement::tex_description, deleter>;

		bool allocate(TexUsage usgae, FormatPixel format, tex_size size, uint64_t target_mipmap, const std::byte* data);
		bool allocate(TexUsage usgae, Tool::asset::id id);
		operator bool() const noexcept { return ptr; }
		bool create(Tool::asset::id, TexUsage usgae = TexUsage::UNCHANGE);
		texture() = default;
		texture(texture&& t) = default;
		//texture(const texture& t);
		texture& operator= (texture&&);
		//texture& operator= (const texture&);

	private:
		description_ptr ptr;
	};


	namespace Resource
	{

		struct vertex_layout
		{
			std::string semantics;
			size_t offset;
			FormatPixel format;
		};

		namespace Implement
		{
			template<typename index_t, typename tuple_type> struct offset_calculate;
			template<size_t ...index_t, typename tuple_type> struct offset_calculate<std::index_sequence<index_t...>, tuple_type>
			{
				std::vector<uint32_t> operator()() const noexcept
				{
					tuple_type* ptr = nullptr;
					return { static_cast<uint32_t>(
						(reinterpret_cast<const std::byte*>(&std::get<index_t>(*ptr))
						- reinterpret_cast<const std::byte*>(ptr)
						))... };
				}
			};

			std::vector<vertex_layout> link(const std::vector<const char*>&, const std::vector<uint32_t>& offset, const std::vector<FormatPixel>& format);

		}

		struct vertex_resource : resource_id
		{
			
			const vertex_layout* layout_ptr() const noexcept { return m_layout_ptr.data(); }
			size_t layout_buffer_space() const noexcept { return sizeof(vertex_layout) * m_layout_ptr.size(); }
			size_t layout_count() const noexcept { return m_layout_ptr.size(); }

			const std::byte* vertex_ptr() const noexcept { return  m_vertex_ptr.get(); }
			size_t vertex_buffer_space() const noexcept { return m_vertex_buffer_space; }
			size_t vertex_count() const noexcept { return m_vertex_count; }

			FormatPixel index_format() const noexcept { return m_index_format; }
			const std::byte* index_ptr() const noexcept { return m_index_ptr.get(); }
			size_t index_count() const noexcept { return m_index_count; }
			size_t index_buffer_space() const noexcept { return calculate_pixel_size(index_format()) * index_count(); }

			template<typename index_type, typename ...vertex_type>
			static resource_owner_ptr<vertex_resource> create(
				size_t index_count, const index_type* index_buffer, size_t vertex_count,
				const std::vector<const char*>& semantics, const std::tuple<vertex_type...>* vertex_init
			)
			{
				assert(semantics.size() == sizeof...(vertex_type));
				return create(as_format<index_type>::value, index_count, reinterpret_cast<const std::byte*>(index_buffer),
					vertex_count, sizeof(std::tuple<vertex_type...>),
					Implement::link(semantics, Implement::offset_calculate<std::make_index_sequence<sizeof...(vertex_type)>, std::tuple<vertex_type...>>{}(), { as_format<vertex_type>::value... }),
					reinterpret_cast<const std::byte*>(vertex_init)
				);
			}

			static resource_owner_ptr<vertex_resource> create(
				FormatPixel index_format, size_t index_count, const std::byte* index_buffer,
				size_t vertex_count, size_t vertex_width, std::vector<vertex_layout> layout,
				const std::byte* vertex_buffer
			);

		private:
			
			vertex_resource();

			std::vector<vertex_layout> m_layout_ptr;
			//size_t m_layout_count;

			std::unique_ptr<std::byte[]> m_vertex_ptr;
			size_t m_vertex_buffer_space;
			size_t m_vertex_count;
			size_t m_vertex_width;

			FormatPixel m_index_format;
			std::unique_ptr<std::byte[]> m_index_ptr;
			size_t m_index_buffer_space;
			size_t m_index_count;
		};
	}
	*/



	/*
	struct vertexs
	{

	};








	struct vertex_desc
	{
		const char* semantics;
		size_t solt;
		size_t offset;
		FormatPixel format;
	};

	struct alignas(vertex_desc) grid_resource
	{
		const size_t element_count;
		const size_t vertex_count;
		const size_t vertex_buffer_size;
		const FormatIndex index_format;
		const size_t index_count;
	};

	namespace Implement
	{

		// grid * vertex_desc ... * data...
		struct alignas(grid_resource) grid_desc : Tool::atomic_reference_count
		{
			std::atomic_bool avalible;
		};
	}


	struct grid
	{
		struct deleter
		{
			void operator()(Implement::grid_desc* gd) const noexcept;
		};
		using description_ptr = Tool::intrusive_ptr<Implement::grid_desc, deleter>;
		description_ptr ptr;
	public:
		void create_grid(FormatIndex index, size_t Index_count, std::function<void(std::byte*)> index_fun,
			size_t vertex_count, std::initializer_list<std::pair<const char*, FormatPixel>> il,
			std::function<void(std::byte*)> vertex_fun
			);
		operator bool() const noexcept {return ptr; }
		const grid_resource& resource() const noexcept { return *reinterpret_cast<const grid_resource*>(ptr.ptr() + 1); }
		const vertex_desc* description() const noexcept { return reinterpret_cast<const vertex_desc*>(&resource() + 1); }
		const std::byte* vertex_buffer() const noexcept { 
			return reinterpret_cast<const std::byte*>(reinterpret_cast<const vertex_desc*>(&resource() + 1) + resource().element_count);
		}
		const std::byte* index_buffer() const noexcept {
			return vertex_buffer() + resource().vertex_buffer_size;
		}
	};
	*/

	




	/*

	struct tscf_description
	{
		std::u16string pattern;
		uint32_t blend_mode;
	};

	struct tscf_description_analyzer
	{

	};
	*/

	/*
	struct tscf_resource : Tool::asset_interface
	{
		const size_t code_count;
		const char* buffer() const noexcept;
	};
	*/

	//Tool::asset_interface* tscf_loader(const Tool::asset::description& dec);


//}