//
// Created by anton on 29.09.17.
//

#include <string>
#include <mutex>
#include <thread>

#include "gtest/gtest.h"
#include "simple_di.h"
namespace {
    constexpr size_t NUM_PARAMS_MAX = 100;
}

class test_class_with_1_arg {
public:
    INJECT(test_class_with_1_arg, int) {};
};
class test_class_with_2_arg {
public:
    INJECT(test_class_with_2_arg, int, float) {};
};
class test_class_with_3_arg {
public:
    INJECT(test_class_with_3_arg, int, float, double) {};
};
class test_class_with_0_arg {
public:
    INJECT_EMPTY(test_class_with_0_arg) {};
};

class test_shared_class {
};

class test_resolve_shared {
public:
    int a;
    std::shared_ptr<test_shared_class> t;
    INJECT(test_resolve_shared, int a, std::shared_ptr<test_shared_class> p) {
        this->a = 1682;
        t = p;
    }
};

class test {
public:
    INJECT(test, int a) : a_(a) {}
    int a_;
    operator int() {return a_;}
};
class dependant {
public:
    INJECT(dependant, test t, int a) : t_(t), a_(a) {}
    test t_;
    int a_;
    operator int() {return t_;}
    int get_test() { return a_;}
};
class another_dependant {
public:
    INJECT(another_dependant, dependant d) : d_(d){}
    dependant d_;
    int test_int() { return d_.get_test();}
};

TEST(DI_TESTS, POD_TYPES) {
    di::Injector test_injector;
    int int_val = 123;
    float float_val = 123;
    double double_val = 123;
    long long_val = 123;
    short short_val = 123;
    char char_val = 'a';
    test_injector.registrate<int>(&int_val);
    test_injector.registrate<float>(&float_val);
    test_injector.registrate<double>(&double_val);
    test_injector.registrate<long>(&long_val);
    test_injector.registrate<short>(&short_val);
    test_injector.registrate<char>(&char_val);
    ASSERT_EQ(test_injector.resolveValue<int>(), int_val);
    ASSERT_EQ(test_injector.resolveValue<float>(), float_val);
    ASSERT_EQ(test_injector.resolveValue<double>(), double_val);
    ASSERT_EQ(test_injector.resolveValue<long>(), long_val);
    ASSERT_EQ(test_injector.resolveValue<short>(), short_val);
    ASSERT_EQ(test_injector.resolveValue<char>(), char_val);
}

TEST(DI_TESTS, COMPLEX_TYPES) {
    di::Injector test_injector;
    std::string test_str("test");
    test_injector.registrate<std::string>(&test_str);
    ASSERT_EQ(*test_injector.resolve<std::string>(), test_str);
}

TEST(DI_TESTS, BY_VALUE) {
    di::Injector test_injector;
    std::string test_str("test");
    test_injector.registrate<std::string>(&test_str);
    ASSERT_EQ(test_injector.resolveValue<std::string>(), test_str);
}

TEST(DI_TESTS, BY_SHARED) {
    di::Injector test_injector;
    std::string test_str("test");
    test_injector.registrate<std::string>(&test_str);
    ASSERT_EQ(*test_injector.resolve<std::string>(), test_str);
}

TEST(DI_TESTS, TEST_AUTO_RESOLVE) {
    constexpr int test_int = 911;
    {
        di::Injector test_injector;
        test_injector.registrate<int>(new int(test_int));
        test_injector.registrate<test>();
        test_injector.registrate<dependant>();
        auto d = test_injector.resolve<dependant>();
        ASSERT_EQ(test_int, static_cast<int>(*d));
        ASSERT_EQ(test_int, d->get_test());
    }
    {
        di::Injector test_injector;
        test_injector.registrate<int>(new int(test_int));
        test_injector.registrate<test>();
        test_injector.registrate<dependant>();
        test_injector.registrate<another_dependant>();
        auto d = test_injector.resolve<dependant>();
        auto a_d = test_injector.resolve<another_dependant>();
        ASSERT_EQ(test_int, a_d->test_int());
    }
}

TEST(DI_TESTS, TEST_MULTIPLERESOLVES_RETURN_SAME) {
    constexpr int test_int = 911;
    di::Injector test_injector;
    test_injector.registrate<int>(new int(test_int));
    test_injector.registrate<test>();
    test_injector.registrate<dependant>();
    ASSERT_EQ(test_injector.resolve<dependant>().get(), test_injector.resolve<dependant>().get());
}

TEST(DI_TESTS, TEST_REGISTER_SHARED) {
    constexpr int test_int = 911;
    std::shared_ptr<int> p(new int(test_int));
    di::Injector test_injector;
    test_shared_class* tsc = new test_shared_class;
    std::shared_ptr<test_shared_class> tsc_s(tsc);
    test_injector.registrateShared(p);
    test_injector.registrateShared(tsc_s);
    test_injector.registrate<test_resolve_shared>();
    ASSERT_EQ(test_int, test_injector.resolveValue<int>());
    ASSERT_EQ(tsc, test_injector.resolve<test_shared_class>().get());
    ASSERT_EQ(1682, test_injector.resolve<test_resolve_shared>()->a);
    ASSERT_EQ(tsc, test_injector.resolve<test_resolve_shared>()->t.get());
}

class test_lazy {
public:
    INJECT(test_lazy, di::LazyPtr<dependant> d) : d_(std::move(d)) {}

    di::LazyPtr<dependant> d_;
};

TEST(DI_TESTS, TEST_LAZY) {
    constexpr int test_int = 911;
    di::Injector test_injector;
    test_injector.registrate<int>(new int(test_int));
    test_injector.registrate<test>();
    test_injector.registrate<dependant>();
    test_injector.registrate<test_lazy>();

    std::shared_ptr<test_lazy> l = test_injector.resolve<test_lazy>();
    ASSERT_EQ(l->d_.get().get(), test_injector.resolve<dependant>().get());
    ASSERT_EQ(test_injector.resolve<dependant>().get(), l->d_.get().get());
    ASSERT_EQ(l->d_.get()->a_, test_int);
}

