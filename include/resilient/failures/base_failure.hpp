#pragma once

#include <tuple>

namespace resilient {

template<typename ... FailureTypes>
struct FailureDetectorTag
{
    using failure_types = std::tuple<FailureTypes...>;
};

}