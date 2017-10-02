//
// Created by anton on 29.09.17.
//

#include <string>

#include "gtest/gtest.h"
#include "simple_di.h"

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

