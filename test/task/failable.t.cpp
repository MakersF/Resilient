#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <type_traits>

#include <resilient/task/failable.hpp>

#include <common/utility.t.hpp>

using namespace resilient;

namespace {

struct Value
{
};

struct Error
{
};

struct Error2
{
};

template<typename T>
struct Callable
{
    MOCK_METHOD0_T(call, T());

    T operator()() { return call(); }
};

template<typename T>
using StrictCallable = ::testing::StrictMock<Callable<T>>;

} // namespace

TEST(Failable_get_failure, EnsureCorrectReturnType)
{
    Failable<Error, Value> failable{Error()};
    static_assert(std::is_same<decltype(get_failure(failable)), Error&>::value,
                  "Wrong get_failure return type");
    static_assert(
        std::is_same<decltype(get_failure(test::as_const(failable))), const Error&>::value,
        "Wrong get_failure return type");
    static_assert(std::is_same<decltype(get_failure(std::move(failable))), Error>::value,
                  "Wrong get_failure return type");
}

TEST(Failable_get_value, EnsureCorrectReturnType)
{
    Failable<Error, Value> failable{Error()};
    static_assert(std::is_same<decltype(get_value(failable)), Value&>::value,
                  "Wrong get_value return type");
    static_assert(std::is_same<decltype(get_value(test::as_const(failable))), const Value&>::value,
                  "Wrong get_value return type");
    static_assert(std::is_same<decltype(get_value(std::move(failable))), Value>::value,
                  "Wrong get_value return type");
}

TEST(Failable_get_value_or_with_value, ContainsValue)
{
    Failable<Error, int> failable{0};
    int fallback = 1;
    EXPECT_EQ(get_value_or(failable, fallback), 0);
}

TEST(Failable_get_value_or_with_value, ContainsFailure)
{
    Failable<Error, int> failable{Error()};
    int fallback = 1;
    EXPECT_EQ(get_value_or(failable, fallback), fallback);
}

TEST(Failable_get_value_or_with_value, EnsureCorrectReturnType)
{
    Failable<Error, Value> failable{Error()};
    Value fallback;
    static_assert(std::is_same<decltype(get_value_or(failable, fallback)), Value&>::value,
                  "Wrong get_value_or return type");
    // Fallback being less const is fine
    static_assert(std::is_same<decltype(get_value_or(test::as_const(failable), fallback)),
                               const Value&>::value,
                  "Wrong get_value_or return type");
    static_assert(
        std::is_same<decltype(get_value_or(test::as_const(failable), test::as_const(fallback))),
                     const Value&>::value,
        "Wrong get_value_or return type");
    static_assert(std::is_same<decltype(get_value_or(std::move(failable), Value())), Value>::value,
                  "Wrong get_value_or return type");
}

TEST(Failable_get_value_or_with_failable, FirstContainsValue)
{
    Failable<Error, int> failable{0};
    Failable<Error2, int> fallback{2};
    auto result = get_value_or(failable, fallback);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 0);
}

TEST(Failable_get_value_or_with_failable, FirstContainsFailureSecondContainsValue)
{
    Failable<Error, int> failable{Error()};
    Failable<Error2, int> fallback{2};
    auto result = get_value_or(failable, fallback);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 2);
}

TEST(Failable_get_value_or_with_failable, FirstContainsFailureSecondContainsFailure)
{
    Failable<Error, Value> failable{Error()};
    Failable<Error2, Value> fallback{Error2()};
    auto result = get_value_or(failable, fallback);
    EXPECT_TRUE(holds_failure(result));
}

TEST(Failable_get_value_or_with_failable, EnsureCorrectReturnType)
{
    Failable<Error, Value> failable{Error()};
    Failable<Error2, Value> fallback{Error2()};
    static_assert(
        std::is_same<decltype(get_value_or(failable, fallback)), Failable<Error2, Value>>::value,
        "Wrong get_value_or return type");
    static_assert(std::is_same<decltype(get_value_or(test::as_const(failable), fallback)),
                               Failable<Error2, Value>>::value,
                  "Wrong get_value_or return type");
    static_assert(
        std::is_same<decltype(get_value_or(test::as_const(failable), test::as_const(fallback))),
                     Failable<Error2, Value>>::value,
        "Wrong get_value_or return type");
    static_assert(std::is_same<decltype(get_value_or(std::move(failable), fallback)),
                               Failable<Error2, Value>>::value,
                  "Wrong get_value_or return type");
    static_assert(std::is_same<decltype(get_value_or(std::move(failable), std::move(fallback))),
                               Failable<Error2, Value>>::value,
                  "Wrong get_value_or return type");
}

TEST(Failable_get_value_or_invoke_with_value, ContainsValue)
{
    Failable<Error, int> failable{0};
    StrictCallable<int&> callable;
    EXPECT_CALL(callable, call()).Times(0);
    EXPECT_EQ(get_value_or_invoke(failable, callable), 0);
}

TEST(Failable_get_value_or_invoke_with_value, ContainsFailure)
{
    Failable<Error, int> failable{Error()};
    StrictCallable<int&> callable;
    EXPECT_CALL(callable, call()).WillOnce(::testing::ReturnRefOfCopy(1));
    EXPECT_EQ(get_value_or_invoke(failable, callable), 1);
}

TEST(Failable_get_value_or_invoke_with_value, EnsureCorrectReturnType)
{
    Failable<Error, Value> failable{Error()};
    Callable<Value&> returnRef;
    Callable<const Value&> returnConstRef;
    Callable<Value> returnVal;
    static_assert(std::is_same<decltype(get_value_or_invoke(failable, returnRef)), Value&>::value,
                  "Wrong get_value_or_invoke return type");
    // Fallback being less const is fine
    static_assert(std::is_same<decltype(get_value_or_invoke(test::as_const(failable), returnRef)),
                               const Value&>::value,
                  "Wrong get_value_or_invoke return type");
    static_assert(
        std::is_same<decltype(get_value_or_invoke(test::as_const(failable), returnConstRef)),
                     const Value&>::value,
        "Wrong get_value_or_invoke return type");
    static_assert(
        std::is_same<decltype(get_value_or_invoke(std::move(failable), returnVal)), Value>::value,
        "Wrong get_value_or_invoke return type");
}

TEST(Failable_get_value_or_invoke_with_failable, FirstContainsValue)
{
    Failable<Error, int> failable{0};
    StrictCallable<Failable<Error2, int>> fallback;
    EXPECT_CALL(fallback, call()).Times(0);
    auto result = get_value_or_invoke(failable, fallback);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 0);
}

TEST(Failable_get_value_or_invoke_with_failable, FirstContainsFailureSecondContainsValue)
{
    Failable<Error, int> failable{Error()};
    StrictCallable<Failable<Error2, int>> fallback;
    EXPECT_CALL(fallback, call()).WillOnce(::testing::Return(Failable<Error2, int>(2)));
    auto result = get_value_or_invoke(failable, fallback);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 2);
}

TEST(Failable_get_value_or_invoke_with_failable, FirstContainsFailureSecondContainsFailure)
{
    Failable<Error, Value> failable{Error()};
    StrictCallable<Failable<Error2, Value>> fallback;
    EXPECT_CALL(fallback, call()).WillOnce(::testing::Return(Failable<Error2, Value>(Error2())));
    auto result = get_value_or_invoke(failable, fallback);
    EXPECT_TRUE(holds_failure(result));
}

TEST(Failable_get_value_or_invoke_with_failable, EnsureCorrectReturnType)
{
    Failable<Error, Value> failable{Error()};
    Callable<Failable<Error2, Value>&> returnRef;
    Callable<const Failable<Error2, Value>&> returnConstRef;
    Callable<Failable<Error2, Value>> returnVal;
    static_assert(std::is_same<decltype(get_value_or_invoke(failable, returnRef)),
                               Failable<Error2, Value>>::value,
                  "Wrong get_value_or_invoke return type");
    // Fallback being less const is fine
    static_assert(std::is_same<decltype(get_value_or_invoke(test::as_const(failable), returnRef)),
                               Failable<Error2, Value>>::value,
                  "Wrong get_value_or_invoke return type");
    static_assert(
        std::is_same<decltype(get_value_or_invoke(test::as_const(failable), returnConstRef)),
                     Failable<Error2, Value>>::value,
        "Wrong get_value_or_invoke return type");
    static_assert(std::is_same<decltype(get_value_or_invoke(std::move(failable), returnVal)),
                               Failable<Error2, Value>>::value,
                  "Wrong get_value_or_invoke return type");
}
