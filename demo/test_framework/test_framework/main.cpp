#include <map>

struct A {};





int main()
{
	A a;
	std::map<int, A&> map12 = { {1, a}, {2,a}, {3, a} };

}