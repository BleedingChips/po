#include "po/po_implement.h"
#include <iostream>
struct component_1 : PO::component_res {};
struct component_2 : PO::component_res {};

std::mutex cout_mutex;

struct system_1 : PO::system_res
{
	void operator()(PO::context& c, component_1& c1) {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		std::cout << "call system_1 : " << std::this_thread::get_id() << std::endl;
	}
};

struct system_2 : PO::system_res
{
	void operator()(PO::context& c, component_2& c1) {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		std::cout << "call system_2 : " << std::this_thread::get_id() << std::endl;
	}
};

struct system_3 : PO::system_res
{
	size_t count = 0;
	void operator()() {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		std::cout << "call system_3 : " << std::this_thread::get_id() << std::endl;
	}
	void end(PO::context& c) {
		++count;
		if (count > 4)
			c.close_context();
	}
};

int main()
{
	PO::context_implement imp;
	imp.create([](PO::context& c) {
		auto ent = c.create_entity();
		c.create_component<component_1>(ent);
		c.create_component<component_2>(ent);
		c.create_system<system_1>();
		c.create_system<system_2>();
		c.create_system<system_3>();
	});
	imp.set_duration(PO::duration_ms{ 2000 });
	imp.loop();
	system("pause");
	return 0;
}