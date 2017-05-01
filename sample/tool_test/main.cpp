#include <iostream>
using namespace std;
#include "../../tool/tmpcall.h"
#include "../../tool/tool.h"
#include "../../tool/auto_adapter.h"
#include "../../tool/thread_tool.h"
#include "../../frame/frame.h"


using namespace PO::TmpCall;
template<size_t i> using si = std::integral_constant<size_t, i>;
template<char i> using ci = std::integral_constant<char, i>;

template<typename ...T> struct op {};

template<typename T, typename K> struct lable_size
{
	using type = std::integral_constant<size_t, sizeof(T)>;
};

struct A { 
	A(int) { cout << "A()" << endl; } 
	~A() { cout << "~A()" << endl; } 
	A(const A&) { cout << "A(A)" << endl; }
	void call() {}
};
struct B { 
	B() { cout << "B()" << endl; } 
	~B() { cout << "~B()" << endl; } 
	B(const B&) { cout << "B(B)" << endl; }
	void call() {}
};
struct C { 
	C() { cout << "C()" << endl; } 
	~C() { cout << "~C()" << endl; } 
	C(const C&) { cout << "C(C)" << endl; }
};

template<typename T> struct is_A
{
	using type = std::is_same<T, A>;
};

struct OPC
{
	template<typename ...T>
	struct in
	{
		using type = int;
	};
};

template<typename ...T> struct Y {};

void FUNC(C, B, A) { cout << "call FUNC!" << endl; }

struct adapter
{
	template<typename T, typename K> using match = std::is_convertible<T, K>;
	template<typename ...T> struct combine
	{
		using type = Y<T...>;
	};
};

struct alignas(4) poi
{

};

struct TXT
{

};

struct TXT2
{

};

template<typename T, typename K, typename F>
auto lock_scope_look(T&& t, K&& k, F&& f)
{
	return t.lock([&f, &k](auto&& tt) mutable {
		return k.lock([&tt, &f](auto&& kk) mutable {
			return f(tt, kk);
		});
	});
}

struct A1 {};
struct A2 {};

struct text_plugin
{
	text_plugin(PO::self_depute<A1, A2> po) 
	{
		cout << "create text_plugin" << endl;
	}
	text_plugin(PO::peek<A1, A2> c) { 
		cout << "FUCK!!" << endl; 
		c.self.bind_tick([](PO::self_depute<A1, A2>, PO::duration da) { cout << "functin" << endl;  });
	}
	~text_plugin()
	{
		cout << "~ text_plugin" << endl;
	}
};

int main()
{
	{
		A1 a1;
		A2 a2;
		//PO::Implement::plugins<A1, A2> pl;

		PO::Implement::plugins<A1, A2> pl;
		PO::Implement::plugins<void, A2>& pll = pl;
		pl.outside_create(PO::plugin_t<text_plugin>{}, a2);
	}
	







	PO::raw_scene rs;
	std::fstream f("out.txt", std::ios::out);

	
	PO::io_task_instance().set_function(
		typeid(TXT),
		[](PO::io_block ib) -> PO::Tool::any
	{
		std::cout << "call TXT func" << std::endl;
		return A{1};
	}
	);

	PO::io_task_instance().set_function(
		typeid(TXT2),
		[](PO::io_block ib) -> PO::Tool::any
	{
		std::cout << "call TXT2 func" << std::endl;
		return B{};
	}
	);

	cout << "====" << endl;

	rs.find(typeid(TXT), u"in.txt", PO::Tool::any{}, true);

	system("pause");
}