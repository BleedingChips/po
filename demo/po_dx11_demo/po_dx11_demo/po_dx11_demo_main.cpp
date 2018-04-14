#include "../../../include/po/po_implement.h"
#include "../../../include/graphic/dx11/dx11_form.h"
#include "../../../include/graphic/dx/hlsl_reflection.h"
#include "..\..\..\..\DirectXTex\DirectXTex\DirectXTex.h"
#include <iostream>

using namespace PO;
using namespace PO::Dx11;
using namespace PO::Dx;
using namespace PO::DXGI;

class debug_view : public PO::Dx11::view_interface
{
	PO::Dx11::view_projection view;
public:
	debug_view() : view(float2{ 1024, 768 }) {}
	virtual void render(PO::Dx11::stage_context& sc, PO::Dx11::tex2& back) override
	{
		PO::Dx11::output_merge_stage oms;
		render_target_view<tex2> uav = back.cast_render_target_view(sc.dev);
		oms << uav;
		sc << oms;
		sc.clear_render_target(oms, {0.5, 0.5, 0.5, 0.5});
	}
};

struct debug_view_handle
{
	std::shared_ptr<debug_view> view;
	debug_view_handle() : view(std::make_shared<debug_view>()) {}
	void close() { view.reset(); }
};

struct testing_form
{
	PO::Dx11::form form;
	debug_view_handle handle;
	testing_form() { form.create_form();  form.insert_view(handle.view); }
	void close() { handle.close(); form.close(); }
};

struct event_picker_system : public PO::system_default
{
	std::vector<event> event_pool;
	void start(PO::context& c) {}
	void operator()(PO::context& c, PO::pre_filter<testing_form> form)
	{

		form << [&](entity e, testing_form& form) {
			std::array<event, 20> event_pool;
			size_t size = form.form.pop_event(event_pool.data(), 20);
			for (size_t i = 0; i < size; ++i)
			{
				if (event_pool[i].is_quit())
				{
					form.close();
					//form.destory();
					c.close_context();
				}

			}
		};
	}
	void end(PO::context& c)
	{
	}
};


int main()
{
	/*
	auto current = ptr + 28;
	uint32_t chunk_count = *(uint32_t*)current;
	current += sizeof(uint32_t);
	std::cout << chunk_count << std::endl;
	for (size_t i = 0; i < chunk_count; ++i)
	{
		uint32_t chunk_start = *(uint32_t*)current;
		std::cout << chunk_start << std::endl;
		current += sizeof(uint32_t);
		auto current_chunk = ptr + chunk_start;
		std::string da = { current_chunk, 4 };
		std::cout << da << std::endl;
		auto chunk_data_start = (current_chunk += 8);
		if (da == "RDEF")
		{
			current_chunk += 8;
			uint32_t resource_binding_count = *(uint32_t*)current_chunk;
			std::cout << "resource binding : " << resource_binding_count << std::endl;
			current_chunk += 4;
			uint32_t resource_byte_offset = *(uint32_t*)current_chunk;
			std::cout << "resource binding offset : " << resource_byte_offset << std::endl;
			current_chunk = chunk_data_start + resource_byte_offset;
			for (size_t i = 0; i < resource_binding_count; ++i)
			{
				resource_bind_block rbb = *(resource_bind_block*)current_chunk;
				current_chunk += sizeof(resource_bind_block);
				auto name_start = chunk_data_start + rbb.offset;
				size_t index = 0;
				for (; name_start[index] != 0; ++index);
				std::string da(name_start, index);
				std::cout << da << std::endl;
			}
			break;
		}
	}
	*/

	/*
	std::cout << chunk_count << std::endl;
	for (size_t i = 0; i < chunk_count; ++i)
	{
		uint32_t chunk_adress;
		file.read((char*)(&chunk_adress), 4);
		std::streampos this_chunk_pos = chunk_adress;
		auto current_poi = file.tellg();
		file.seekg(this_chunk_pos, std::ios::beg);
		char data[5];
		file.read(data, 4);
		data[4] = 0;
		std::cout << data << std::endl;
		std::string da = data;
		if (da == "RDEF")
		{
			uint32_t length;
			file.read((char*)(&length), sizeof(uint32_t));
			auto cur_poi = file.tellg();
			RDEF_chunk_scription scri;
			file.read((char*)(&scri), sizeof(RDEF_chunk_scription));
			std::cout << "resource binding :" << scri.resource_binding_count << std::endl;
			
			cur_poi = cur_poi + decltype(cur_poi){scri.resource_byte_offset};
			file.seekg(cur_poi);
			uint32_t type_set;
			file.read((char*)(&type_set), sizeof(uint32_t));

		}
		file.seekg(current_poi, std::ios::beg);
	}
	*/

	if(true)
	{
		PO::context_implement imp;
		imp.set_duration( PO::duration_ms{10} );

		imp.init([](PO::context& c) {
			auto e = c.create_entity();
			c.create_component<testing_form>(e);
			c.create_system<event_picker_system>();
		});

		imp.loop();
	}
	std::cout << "asdasd" << std::endl;
	system("pause");
	return 0;

}

