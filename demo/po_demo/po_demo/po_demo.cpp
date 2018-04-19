#include "po/po_implement.h"
#include "po/frame/context.h"
#include <iostream>

// debug component
struct component_1 { ~component_1() { std::cout << "~component_1" << std::endl; } };
struct component_2 { ~component_2() { std::cout << "~component_2" << std::endl; } };

std::mutex cout_mutex;

struct event_1
{
	int generator;
	PO::entity creating_entity;
};

struct system_1 : PO::system_default
{
	void operator()(PO::context& c, component_1& c1, PO::provider<event_1> p) {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		std::cout << "call system_1 in thread:" << std::this_thread::get_id() << std::endl;
		// generator empty event
		p.push_back(event_1{ 1, {} });
	}
	~system_1() { 
		std::cout << "destory system1" << std::endl; 
	}
};

struct system_2 : PO::system_default
{
	// catch all entity have component_2 and generator event_1
	void operator()(PO::context& c, PO::pre_filter<const component_2> pf, PO::provider<event_1> p) {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		std::cout << "call system_2 in thread:" << std::this_thread::get_id() << std::endl;
		//clear all entity create form system_2
		p.clear();
		//call all entity have component_2 and generator event
		size_t call_count = pf << [&](PO::entity e, const component_2& c)
		{
			p.push_back(event_1{ 2, e });
		};
	}
	static PO::SystemLayout layout() noexcept {
		// advoid confuse with system_1 because of PO::provider<event_1>, with 
		return PO::SystemLayout::PRE_UPDATE;
	}
};

struct system_3 : PO::system_default
{
	size_t count;
	//capture singleton component_2 and receiver all event_1, and ready to capture component_2 form entity
	void operator()(PO::context& c, component_2& c1, PO::receiver<event_1> r, PO::filter<const component_2> f) {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		std::cout << "call system_3 in thread: " << std::this_thread::get_id() << std::endl;
		size_t event_count = r << [&](const event_1& e)
		{
			std::cout << "this event come form " << e.generator;
			if (f[e.creating_entity] << [](PO::entity&, const component_2&){ 
				// do nothing here.
			})
				std::cout << " this entity have component_2";
			std::cout << std::endl;
		};
		++count;
		if (count >= 10)
			// close context;
			c.close_context();
	}
};

// just call once
struct temporary_system : PO::system_default
{
	void operator()(PO::context& c, PO::provider<event_1> p) {
		std::lock_guard<decltype(cout_mutex)> lg(cout_mutex);
		std::cout << "call temporary_system in thread : " << std::this_thread::get_id() << std::endl;
		//p.clear();
		p.push_back(event_1{ 4 });
		//c.close_context();
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
	}
};

int main()
{
	
	//std::cout << ptr->info().write.operator[](0).name() << std::endl;
	
	PO::context_implement imp;
	imp.init([](PO::context& c) {
		//create temporary_system
		c.create_temporary_system<temporary_system>();
	});
	imp.set_duration(PO::duration_ms{ 5000 });
	//imp.set_thread_reserved(100);
	imp.loop();
	system("pause");
	return 0;
}