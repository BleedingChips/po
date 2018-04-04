#include "po/po.h"
#include "po/frame/context.h"
#include <iostream>
/*
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
*/

struct A {};
struct B {};
struct C {};
struct D {};

struct system_testing : PO::system_default_define
{
	void operator()(PO::context& c, PO::filter<A&> f) {}
};

int main()
{
	using namespace PO::ECSFramework;

	using type = typename PO::ECSFramework::Implement::system_requirement_detect<PO::ECSFramework::context&, filter<A&, const B&>, filter<const A&, const C&, A&>, A& , const B&, D&>::type;
	std::cout << typeid(typename type::filter::write).name() << std::endl;
	std::cout << typeid(typename type::filter::read).name() << std::endl;
	std::cout << typeid(typename type::singleton::write).name() << std::endl;
	std::cout << typeid(typename type::singleton::read).name() << std::endl;
	std::cout << PO::ECSFramework::Implement::is_const<A&>::value << std::endl;

	auto ptr = std::make_unique<PO::ECSFramework::Implement::system_implement<system_testing>>();
	//std::cout << ptr->info().write.operator[](0).name() << std::endl;


	


	/*
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
	*/
	system("pause");
	return 0;
}