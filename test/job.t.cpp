#include <resilient/job.hpp>
#include <resilient/task/task.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <type_traits>
#include <iostream>

using namespace resilient;

TEST(test, test4)
{
    RetryPolicy rp{3};
    auto fun = []() { std::cout << "A" << std::endl; return 1;};
    //auto ret = job().with(rp).run(task(fun));
    auto ret = rp(task(fun));
    //decltype(ret)::aa;
}