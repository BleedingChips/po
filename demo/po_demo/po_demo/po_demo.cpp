#include "po/po_implement.h"
#include "po/frame/context.h"
#include <iostream>

struct component_1 { ~component_1() { std::cout << "~component_1" << std::endl; } };
struct component_2 { 
	~component_2() { 
		std::cout << "~component_2" << std::endl; 
	} 
};

std::mutex cout_mutex;

struct event_1
{
	int id;
};

struct system_1 : PO::system_default
{
	void operator()(PO::context& c, component_1& c1, PO::provider<event_1> p) {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		std::cout << "call system_1 : " << std::this_thread::get_id() << std::endl;
		p.clear();
		p.push_back(event_1{1});
	}
	~system_1() { 
		std::cout << "destory system1" << std::endl; 
	}
};

struct system_2 : PO::system_default
{
	void operator()(PO::context& c, PO::pre_filter<component_2> f, component_2& c1, PO::receiver<event_1> r) {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		f << [&](PO::entity e, component_2& da) {
			//std::cout << "1" << std::endl;
			//c.destory_component<component_2>(e);
			//c.destory_system<system_1>();
			//c.destory_singleton_component<component_1>();
		};

		r << [](const event_1& t) { /*std::cout << "event id" << t.id << std::endl*/;  };

		std::cout << "call system_2 : " << std::this_thread::get_id() << std::endl;
	}
};

struct system_3 : PO::system_default
{
	std::size_t count = 0;
	void operator()(PO::context& c, PO::provider<event_1> p) {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		std::cout << "call system_3 : " << std::this_thread::get_id() << std::endl;
		p.clear();
		p.push_back(event_1{3});
		//c.close_context();
	}
	static PO::SystemLayout layout() noexcept { 
		// advoid confuse with system_1 because of PO::provider<event_1>
		return PO::SystemLayout::PRE_UPDATE;
	}
};


int main()
{
	
	//std::cout << ptr->info().write.operator[](0).name() << std::endl;
	
	PO::context_implement imp;
	imp.init([](PO::context& c) {
		auto ent = c.create_entity();
		c.create_component<component_1>(ent);
		c.create_component<component_2>(ent);
		auto ent2 = c.create_entity();
		c.create_component<component_1>(ent2);
		c.create_component<component_2>(ent2);
		c.create_singleton_component<component_1>();
		c.create_singleton_component<component_2>();
		c.create_system<system_1>();
		c.create_system<system_2>();
		c.create_system<system_3>();
	});
	imp.set_duration(PO::duration_ms{ 2000 });
	//imp.set_thread_reserved(100);
	imp.loop();
	system("pause");
	return 0;
}