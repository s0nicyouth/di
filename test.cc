#include <iostream>
#include <typeinfo>
#include "simple_di.h"

class test_class {
public:
	INJECT(test_class, std::shared_ptr<int> a, std::shared_ptr<float> b) {};
	void test() {std::cout << "Called test_class" << std::endl;}
};

class test_dependant {
public:
	INJECT(test_dependant, std::shared_ptr<test_class> t, int a) { std::cout << "Created dependant. a = " << a << std::endl;}
	void test() {std::cout << "Called dependant" << std::endl;}
};

class Interface {
public:
	virtual void test() {std::cout << "Interface" << std::endl;}
};
class Implementation : public Interface {
public:
	INJECT(Implementation, std::shared_ptr<test_dependant> t) {}
	void test() override {std::cout << "Implementation" << std::endl;}
};

int main() {
	di::Injector i;
	di::Injector j;
	int ii = 17;
	int jj = 27;
	float ff = 17;
	j.Register<int>(&jj);
	i.RegisterInterface<Interface, Implementation>();
	i.Register<test_dependant>();
	i.Register<test_class>();
	i.Register<int>(&ii);
	i.Register<float>(&ff);
	std::cout << *i.Resolve<int>() << std::endl;
	i.Resolve<Interface>()->test();
	i.Resolve<test_dependant>()->test();
	std::cout << *j.Resolve<int>() << std::endl;
}