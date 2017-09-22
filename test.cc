#include "inject_markers.h"
#include "type_info.h"
#include <iostream>
#include <typeinfo>
#include "type_holder.h"

class test {
public:
	INJECT(test, int* a) {}
};

class test_class {
public:
	INJECT(test_class, test* t, int* a) { std::cout << "Value of a is " << *a << std::endl;}
	/*explicit test_class(int* b, int* a) {}
	explicit test_class(int* b) {}*/
	std::string test() {return "Works";}
};

int f(std::string a, int b) {return 0;};

int main() {
	int* i = new int;
	*i = 17;
	di::Register<int>(i);
	di::Register<test>();
	di::Register<test_class>();
	std::cout << di::Resolve<test_class>()->test() << std::endl;
	//std::cout << IsConstructable<T, const DiMark&&, decltype(Any())>{} << std::endl;
	//std::cout << di::ConstructIfPossible<test_class>()->test() << std::endl;
	return 0;
}