#include <iostream>
using namespace std;

template<typename T> struct op233 
{
	void operator() () { cout << typeid(T).name() << endl; }
};

template<typename T> struct op233<T&>
{
	void operator() () { cout << typeid(T).name()<<"&" << endl; }
};

template<typename ...AT> struct call_AT 
{
	void operator() () {}
};
template<typename T, typename ...AT> struct call_AT<T, AT...> 
{ 
	void operator()() {
		op233<T>{}();
		call_AT<AT...>{}();
	}
};





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

void tet(int) {
	cout << "fuck!!!!" << endl;

};

struct Da
{
	void tet(int) {
		cout << "fuck!!!!" << endl;

	};
};

int main()
{
	PO::Tool::auto_adapter<PO::Tool::unorder_adapt>(tet, 1, 1);

	cout << std::is_convertible<int&, int&>::value << endl;

	auto fu = PO::Tool::auto_bind_function<void(int, int, int), PO::Tool::unorder_adapt>(tet);
	fu(1, 1, 1);

	//Da p;

	//std::function<void(int, int)> fun = PO::Tool::auto_bind_function<void(int, int), PO::Tool::unorder_adapt>(&Da::tet, p);

	//fun(1, 1);







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