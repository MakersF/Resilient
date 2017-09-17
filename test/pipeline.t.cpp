#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "resilient/policies/pipeline.hpp"
#include "resilient/common/result.hpp"

using namespace resilient;

int func(char a, float b)
{
    std::cout << a << " " << b << std::endl;
    return 1;
}

TEST(test, test3)
{
    using ReturnType = ResultTraits<int>::type;
    auto l1 = [](char a, float b) { std::cout << a << " " << b << std::endl; return ReturnType(1); };
    char a = 'W';
    float b = 3.14;
    //LValCall<decltype(l1)&, char&, float&> job(l1, a, b);
    //LValCall<decltype(l1)&, char, float> job(l1, 'Q', 6.28);

    RetryPolicy rp{3};
    CircuitBreak cb;
    Monitor mntr;

    // TODO make it work with free functions
    pipelineOf(rp)
    .then(cb)
    .then(mntr)
    .then(CircuitBreak{true})(l1, a, b);

}