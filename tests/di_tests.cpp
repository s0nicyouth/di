//
// Created by anton on 29.09.17.
//

#include <string>

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
    test_injector.Register<int>(&int_val);
    test_injector.Register<float>(&float_val);
    test_injector.Register<double>(&double_val);
    test_injector.Register<long>(&long_val);
    test_injector.Register<short>(&short_val);
    test_injector.Register<char>(&char_val);
    ASSERT_EQ(test_injector.ResolveValue<int>(), int_val);
    ASSERT_EQ(test_injector.ResolveValue<float>(), float_val);
    ASSERT_EQ(test_injector.ResolveValue<double>(), double_val);
    ASSERT_EQ(test_injector.ResolveValue<long>(), long_val);
    ASSERT_EQ(test_injector.ResolveValue<short>(), short_val);
    ASSERT_EQ(test_injector.ResolveValue<char>(), char_val);
}

TEST(DI_TESTS, COMPLEX_TYPES) {
    di::Injector test_injector;
    std::string test_str("test");
    test_injector.Register<std::string>(&test_str);
    ASSERT_EQ(*test_injector.Resolve<std::string>(), test_str);
}

TEST(DI_TESTS, BY_VALUE) {
    di::Injector test_injector;
    std::string test_str("test");
    test_injector.Register<std::string>(&test_str);
    ASSERT_EQ(test_injector.ResolveValue<std::string>(), test_str);
}

TEST(DI_TESTS, BY_SHARED) {
    di::Injector test_injector;
    std::string test_str("test");
    test_injector.Register<std::string>(&test_str);
    ASSERT_EQ(*test_injector.Resolve<std::string>(), test_str);
}

TEST(DI_TESTS, TEST_AUTO_RESOLVE) {
    constexpr int test_int = 911;
    {
        di::Injector test_injector;
        test_injector.Register<int>(new int(test_int));
        test_injector.Register<test>();
        test_injector.Register<dependant>();
        auto d = test_injector.Resolve<dependant>();
        ASSERT_EQ(test_int, static_cast<int>(*d));
        ASSERT_EQ(test_int, d->get_test());
    }
    {
        di::Injector test_injector;
        test_injector.Register<int>(new int(test_int));
        test_injector.Register<test>();
        test_injector.Register<dependant>();
        test_injector.Register<another_dependant>();
        auto d = test_injector.Resolve<dependant>();
        auto a_d = test_injector.Resolve<another_dependant>();
        ASSERT_EQ(test_int, a_d->test_int());
    }
}

