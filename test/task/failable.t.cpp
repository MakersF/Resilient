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
    Failable<Value, Error> failable{Error()};
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
    Failable<Value, Error> failable{Error()};
    static_assert(std::is_same<decltype(get_value(failable)), Value&>::value,
                  "Wrong get_value return type");
    static_assert(std::is_same<decltype(get_value(test::as_const(failable))), const Value&>::value,
                  "Wrong get_value return type");
    static_assert(std::is_same<decltype(get_value(std::move(failable))), Value>::value,
                  "Wrong get_value return type");
}

TEST(Failable_get_value_or_with_value, ContainsValue)
{
    Failable<int, Error> failable{0};
    int fallback = 1;
    EXPECT_EQ(get_value_or(failable, fallback), 0);
}

TEST(Failable_get_value_or_with_value, ContainsFailure)
{
    Failable<int, Error> failable{Error()};
    int fallback = 1;
    EXPECT_EQ(get_value_or(failable, fallback), fallback);
}

TEST(Failable_get_value_or_with_value, EnsureCorrectReturnType)
{
    Failable<Value, Error> failable{Error()};
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
    Failable<int, Error> failable{0};
    Failable<int, Error2> fallback{2};
    auto result = get_value_or(failable, fallback);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 0);
}

TEST(Failable_get_value_or_with_failable, FirstContainsFailureSecondContainsValue)
{
    Failable<int, Error> failable{Error()};
    Failable<int, Error2> fallback{2};
    auto result = get_value_or(failable, fallback);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 2);
}

TEST(Failable_get_value_or_with_failable, FirstContainsFailureSecondContainsFailure)
{
    Failable<Value, Error> failable{Error()};
    Failable<Value, Error2> fallback{Error2()};
    auto result = get_value_or(failable, fallback);
    EXPECT_TRUE(holds_failure(result));
}

TEST(Failable_get_value_or_with_failable, EnsureCorrectReturnType)
{
    Failable<Value, Error> failable{Error()};
    Failable<Value, Error2> fallback{Error2()};
    static_assert(
        std::is_same<decltype(get_value_or(failable, fallback)), Failable<Value, Error2>>::value,
        "Wrong get_value_or return type");
    static_assert(std::is_same<decltype(get_value_or(test::as_const(failable), fallback)),
                               Failable<Value, Error2>>::value,
                  "Wrong get_value_or return type");
    static_assert(
        std::is_same<decltype(get_value_or(test::as_const(failable), test::as_const(fallback))),
                     Failable<Value, Error2>>::value,
        "Wrong get_value_or return type");
    static_assert(std::is_same<decltype(get_value_or(std::move(failable), fallback)),
                               Failable<Value, Error2>>::value,
                  "Wrong get_value_or return type");
    static_assert(std::is_same<decltype(get_value_or(std::move(failable), std::move(fallback))),
                               Failable<Value, Error2>>::value,
                  "Wrong get_value_or return type");
}

TEST(Failable_get_value_or_invoke_with_value, ContainsValue)
{
    Failable<int, Error> failable{0};
    StrictCallable<int&> callable;
    EXPECT_CALL(callable, call()).Times(0);
    EXPECT_EQ(get_value_or_invoke(failable, callable), 0);
}

TEST(Failable_get_value_or_invoke_with_value, ContainsFailure)
{
    Failable<int, Error> failable{Error()};
    StrictCallable<int&> callable;
    EXPECT_CALL(callable, call()).WillOnce(::testing::ReturnRefOfCopy(1));
    EXPECT_EQ(get_value_or_invoke(failable, callable), 1);
}

TEST(Failable_get_value_or_invoke_with_value, EnsureCorrectReturnType)
{
    Failable<Value, Error> failable{Error()};
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
    Failable<int, Error> failable{0};
    StrictCallable<Failable<int, Error2>> fallback;
    EXPECT_CALL(fallback, call()).Times(0);
    auto result = get_value_or_invoke(failable, fallback);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 0);
}

TEST(Failable_get_value_or_invoke_with_failable, FirstContainsFailureSecondContainsValue)
{
    Failable<int, Error> failable{Error()};
    StrictCallable<Failable<int, Error2>> fallback;
    EXPECT_CALL(fallback, call()).WillOnce(::testing::Return(Failable<int, Error2>(2)));
    auto result = get_value_or_invoke(failable, fallback);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 2);
}

TEST(Failable_get_value_or_invoke_with_failable, FirstContainsFailureSecondContainsFailure)
{
    Failable<Value, Error> failable{Error()};
    StrictCallable<Failable<Value, Error2>> fallback;
    EXPECT_CALL(fallback, call()).WillOnce(::testing::Return(Failable<Value, Error2>(Error2())));
    auto result = get_value_or_invoke(failable, fallback);
    EXPECT_TRUE(holds_failure(result));
}

TEST(Failable_get_value_or_invoke_with_failable, EnsureCorrectReturnType)
{
    Failable<Value, Error> failable{Error()};
    Callable<Failable<Value, Error2>&> returnRef;
    Callable<const Failable<Value, Error2>&> returnConstRef;
    Callable<Failable<Value, Error2>> returnVal;
    static_assert(std::is_same<decltype(get_value_or_invoke(failable, returnRef)),
                               Failable<Value, Error2>>::value,
                  "Wrong get_value_or_invoke return type");
    // Fallback being less const is fine
    static_assert(std::is_same<decltype(get_value_or_invoke(test::as_const(failable), returnRef)),
                               Failable<Value, Error2>>::value,
                  "Wrong get_value_or_invoke return type");
    static_assert(
        std::is_same<decltype(get_value_or_invoke(test::as_const(failable), returnConstRef)),
                     Failable<Value, Error2>>::value,
        "Wrong get_value_or_invoke return type");
    static_assert(std::is_same<decltype(get_value_or_invoke(std::move(failable), returnVal)),
                               Failable<Value, Error2>>::value,
                  "Wrong get_value_or_invoke return type");
}
