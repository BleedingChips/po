#include "../../tool/tmpcall.h"
#include "../../tool/tool.h"
#include "../../tool/auto_adapter.h"
#include "../../tool/thread_tool.h"
#include <iostream>
using namespace std;
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
};
struct B { 
	B() { cout << "B()" << endl; } 
	~B() { cout << "~B()" << endl; } 
	B(const B&) { cout << "B(B)" << endl; }
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

int main()
{
	{
		PO::Tool::variant<A, B, C> va;
		va = A{ 1 };
		cout << "===" << endl;
		va = B{};
		cout << "===" << endl;
		PO::Tool::variant<A, B, C> va2(C{});
		cout << "===" << endl;
		va = va2;
		cout << "===" << endl;
		va = {};
		cout << "===" << endl;
	}

	using type = call<append<A, B, C>, sperate_call<append<C, B, A>, first_match_each<make_func<Y>>, make_func<Y>>, make_func<Y>>;
	using type2 = typename PO::Tool::Implement::analyze_implement<true, PO::Tool::unorder_adapt, decltype(FUNC), A, B, C, C, B, A>::type;
	using type3 = typename PO::Tool::Implement::analyze_implement<false, PO::Tool::order_adapt, decltype(FUNC), A, A, B, C, C, B, A>::type;

	PO::Tool::auto_adapter<PO::Tool::unorder_adapt>(FUNC, A{ 1 }, B{}, C{});

	cout << typeid(decltype(&FUNC)).name() << endl;
	cout << typeid(type3).name() << endl;
	cout << typeid(call < append <A, B, C, A, B> , localizer<make_func_t<is_A>>, self > ).name() << endl;
	system("pause");
}