#include "resource.h"
//#include "../../po/tool/utf.h"
#include <fstream>
#include <iostream>
/*
namespace PO::Graphic
{

	namespace Implement
	{
		std::byte* buffer_const_description::shift_buffer() noexcept { return reinterpret_cast<std::byte*>(this + 1); }
		const std::byte* buffer_const_description::shift_buffer() const noexcept { return reinterpret_cast<const std::byte*>(this + 1); }

		resource_owner_ptr<buffer_const_description> buffer_const_description::create(BufUsage usage, size_t size)
		{
			size = Tool::adjust_alignas_space(size, 128);
			auto ptr = Tool::aligna_buffer<alignof(buffer_const_description)>::allocate<buffer_const_description>(sizeof(buffer_const_description) + size);
			new (ptr) buffer_const_description{};
			ptr->m_size = size;
			ptr->m_usage = usage;

			return resource_owner_ptr<buffer_const_description>{
				ptr,
				[](resource_id* data) noexcept {
					data->~resource_id();
					Tool::aligna_buffer<alignof(buffer_const_description)>::release(static_cast<buffer_const_description*>(data));
				}
			};
		}
	}

	size_t buffer::reallocate_buffer(BufUsage usage, size_t size)
	{
		size = Tool::adjust_alignas_space(size, 128);
		if (m_buffer && m_buffer->usage() == usage && m_buffer->size() >= size)
		{
			m_buffer->update();
			return m_buffer->size();
		}
		m_buffer = Implement::buffer_const_description::create(usage, size);
		m_buffer->add_owner_ref();
		return size;
	}

	size_t tex_size::space() const noexcept
	{
		if (x != 0)
		{
			if (y == 0)
				return x;
			if (z == 0)
				return x * y;
			return x * y * z;
		}
		return 0;
	}

	void texture::deleter::operator()(Implement::tex_description* ptr) noexcept
	{
		ptr->~tex_description();
		delete[] reinterpret_cast<Tool::aligna_buffer<alignof(Implement::tex_description)>*>(this);
	}

	bool texture::allocate(TexUsage usgae, FormatPixel format, tex_size size, uint64_t target_mipmap, const std::byte* data)
	{
		if (ptr)
		{
			ptr->avalible = false;
			ptr.reset();
		}
		size_t buffer_size = 0;
		size_t append_size = calculate_pixel_size(format) * size.space();
		if (append_size != 0)
		{
			if (data != nullptr)
			{
				buffer_size = append_size - append_size % alignof(Implement::tex_description) + alignof(Implement::tex_description);
			}
			size_t allocate_size = sizeof(Implement::tex_description) + buffer_size;
			assert(allocate_size % alignof(Implement::tex_description) == 0);
			auto tem_ptr = new Tool::aligna_buffer<alignof(Implement::tex_raw_resource)>[allocate_size / 16];
			ptr = new (tem_ptr) Implement::tex_description{ usgae, Implement::tex_raw_resource{ format,size, target_mipmap, data != nullptr}};
			std::memcpy(ptr.ptr() + 1, data, buffer_size != 0 ? append_size : 0);
			return true;
		}
		return false;
	}

	bool texture::allocate(TexUsage usgae, Tool::asset::id id)
	{
		if (ptr)
		{
			ptr->avalible = false;
			ptr.reset();
		}
		if (id)
		{
			auto tem_ptr = new Tool::aligna_buffer<alignof(Implement::tex_raw_resource)>[sizeof(Implement::tex_description) / 16];
			ptr = new (tem_ptr) Implement::tex_description{ usgae, id };
			return true;
		}
		return false;
	}

	bool texture::create(Tool::asset::id id, TexUsage usgae)
	{
		allocate(usgae, id);
		return true;
	}
	*/

	/*
	texture::texture(const texture& t)
	{
		if (t)
		{

		}
	}
	*/
/*
	texture& texture::operator= (texture&& t)
	{
		texture tem(std::move(t));
		if (ptr)
			ptr->avalible = false;
		ptr = std::move(tem.ptr);
		ptr->avalible = true;
		return *this;
	}


	namespace Resource
	{

		namespace Implement
		{
			std::vector<vertex_layout> link(const std::vector<const char*>& se, const std::vector<uint32_t>& offset, const std::vector<FormatPixel>& format)
			{
				size_t size = se.size();
				assert(offset.size() == size);
				assert(offset.size() == size);
				std::vector<vertex_layout> tem;
				tem.reserve(size);
				for (size_t i = 0; i < size; ++i)
				{
					tem.push_back(vertex_layout{ se[i], offset[i], format[i] });
				}
				return tem;
			}
		}

		vertex_resource::vertex_resource() {}

		resource_owner_ptr<vertex_resource> vertex_resource::create(
			FormatPixel index_format, size_t index_count, const std::byte* index_buffer,
			size_t vertex_count, size_t vertex_width, std::vector<vertex_layout> layout,
			const std::byte* vertex_buffer
		)
		{
			auto ptr = new vertex_resource{};
			ptr->m_layout_ptr = std::move(layout);
			ptr->m_vertex_count = vertex_count;
			ptr->m_vertex_width = vertex_width;
			ptr->m_vertex_buffer_space = vertex_count * vertex_width;
			ptr->m_vertex_ptr = std::unique_ptr<std::byte[]>(new std::byte[ptr->m_vertex_buffer_space]);
			std::memcpy(ptr->m_vertex_ptr.get(), vertex_buffer, ptr->m_vertex_buffer_space);
			ptr->m_index_count = index_count;
			ptr->m_index_format = index_format;
			ptr->m_index_buffer_space = index_count * calculate_pixel_size(index_format);
			ptr->m_index_ptr = std::unique_ptr<std::byte[]>(new std::byte[ptr->m_index_buffer_space]);
			std::memcpy(ptr->m_index_ptr.get(), index_buffer, ptr->m_index_buffer_space);
			return ptr;
		}
*/



		/*
		resource_owner_ptr<vertex_resource> vertex_resource::create(
			size_t index_count, std::function<void(std::byte*)> f,
			size_t vertex_count, std::initializer_list<std::tuple<const char*, FormatPixel>> vertex_layout, std::function<void(std::byte*)>
		)
		{
			size_t index_buffer_size = index
		}
		*/
	//}











	/*
	void grid::deleter::operator()(Implement::grid_desc* gd) const noexcept
	{
		gd->~grid_desc();
		delete[] reinterpret_cast<Tool::aligna_buffer<alignof(Implement::grid_desc)>*>(gd);
	}

	void grid::create_grid(FormatIndex index, size_t Index_count, std::function<void(std::byte*)> index_fun, size_t vertex_count, std::initializer_list<std::pair<const char*, FormatPixel>> il, std::function<void(std::byte*)> vertex_fun)
	{
		if (ptr)
		{
			ptr->avalible = false;
			ptr.reset();
		}

		size_t scri_count = 0;
		size_t vertex_buffer_count = 0;
		for (auto ite : il)
		{
			scri_count++;
			vertex_buffer_count += calculate_pixel_size(ite.second);
		}

		size_t index_buffer = calculate_pixel_size(index) * Index_count;

		size_t total_size =
			sizeof(Implement::grid_desc)
			+ sizeof(grid_resource)
			+ scri_count * sizeof(vertex_desc)
			+ vertex_buffer_count * vertex_count
			+ index_buffer;

		total_size = Tool::adjust_alignas_space(total_size, alignof(Implement::grid_desc));
		void* da = new Tool::aligna_buffer<alignof(Implement::grid_desc)>[total_size / alignof(Implement::grid_desc)];
		ptr = new (da) Implement::grid_desc{};
		grid_resource* res = reinterpret_cast<grid_resource*>(ptr.ptr() + 1);
		new (res) grid_resource{ scri_count , vertex_count ,vertex_buffer_count * vertex_count, index ,Index_count };
		vertex_desc* des_res = reinterpret_cast<vertex_desc*>(res + 1);
		size_t offset = 0;
		size_t des_index = 0;
		for (auto ite : il)
		{
			new (des_res) vertex_desc{ite.first, des_index , offset , ite.second};
			offset += calculate_pixel_size(ite.second);
			++des_index;
			++des_res;
		}
		auto verbuff = reinterpret_cast<std::byte*>(des_res);
		vertex_fun(verbuff);
		index_fun(verbuff + vertex_buffer_count * vertex_count);
	}
	*/

	/*
	const char* tscf_resource::buffer() const noexcept
	{
		return reinterpret_cast<const char*>(this + 1);
	}
	*/

	/*
	Tool::asset_interface* tscf_loader(const Tool::asset::description& dec)
	{
		std::ifstream file(dec.path, std::ios::binary);
		if (file.good())
		{
			Tool::FileCharFormat format = Tool::detect_bom_with_binary_file(file);
			Tool::locade_binary_file(format, file);
			if (format == Tool::FileCharFormat::UNKNOW)
				format = Tool::FileCharFormat::UTF8;
			uint64_t calculate_space = Tool::calculate_last_space(file);
			if (format == Tool::FileCharFormat::UTF8)
			{
				size_t allocate_space = Tool::adjust_alignas_space(calculate_space, alignof(Tool::asset_interface)) + sizeof(tscf_resource) + sizeof(Tool::asset_interface);
				void* buffer = new Tool::aligna_buffer<alignof(Tool::asset_interface)>[allocate_space / alignof(Tool::asset_interface)];
				auto ptr = reinterpret_cast<std::byte*>(buffer);
				auto result = new (buffer) Tool::asset_interface{ dec.last_update_time, typeid(tscf_resource), ptr + sizeof(Tool::asset_interface), [](Tool::asset_interface * ptr) noexcept {  
					reinterpret_cast<tscf_resource*>(ptr + 1)->~tscf_resource();
					ptr->~asset_interface();
					delete[] reinterpret_cast<Tool::aligna_buffer<alignof(Tool::asset_interface)>*>(ptr);
				} };
				new (ptr + sizeof(Tool::asset_interface)) tscf_resource{ calculate_space };
				uint64_t size = Tool::load_to_utf8(file, calculate_space, format, reinterpret_cast<char*>(ptr + sizeof(Tool::asset_interface) + sizeof(tscf_resource)), calculate_space);
				return result;
			}
			assert(false);
		}
		return nullptr;
	}
	*/


//}