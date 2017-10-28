#pragma once

#include <tuple>
#include <resilient/common/base_variant.hpp>

namespace resilient {

struct NoFailure {};

template<typename ... FailureTypes>
struct FailureDetectorTag
{
    using failure_types = std::tuple<FailureTypes...>;
    using failure = Variant<NoFailure, FailureTypes...>;
};

}