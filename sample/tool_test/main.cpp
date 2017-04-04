
#include "../../tool/tmpcall.h"
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

class A { A() {} };
class B {};
class C {};

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


int main()
{
	cout << typeid(call < append < A, B, C, A, B> , localizer<make_func_t<is_A>>, self > ).name() << endl;
	system("pause");
}