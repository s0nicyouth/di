#include <iostream>
#include <typeinfo>
#include "simple_di.h"

class test {
public:
	INJECT(test, int* a) {}
};

class test_class {
public:
	INJECT(test_class, test* t, int* a) { std::cout << "Value of a is " << *a << std::endl;}
	std::string test() {return "Works";}
};
class l2;
class l1 {
public:
	INJECT(l1, l2* a) {};
};

class l2 {
public:
	INJECT(l2, l1* a) {};
};

class Parent {
public:
	virtual void test() {std::cout << "Parent" << std::endl; }
};
class Derived : public Parent {
public:
	INJECT(Derived, int* a) {};
	void test() override {std::cout << "Derived" << std::endl;}
};

class NotConstTest {
public:
	INJECT_EMPTY(NotConstTest) {};
};

int f(std::string a, int b) {return 0;};

int main() {
	int* i = new int;
	*i = 17;
	di::Register<int>(i);
	di::Register<test>();
	di::Register<test_class>();
	di::RegisterInterface<Parent, Derived>();
	di::Register<l1>();
	di::Register<l2>();
	di::Register<NotConstTest>();
	std::cout << "Resolving l1" << std::endl;
	di::Resolve<l1>();
	std::cout << di::Resolve<test_class>()->test() << std::endl;
	di::Resolve<Parent>()->test();
	di::Resolve<NotConstTest>();
	return 0;
}