#pragma once

#include <tuple>
#include <resilient/common/base_variant.hpp>

namespace resilient {

struct NoFailure {};

template<typename ...FailureTypes>
struct FailureDetectorTag
{
    using failure_types = std::tuple<FailureTypes...>;
    using failure = Variant<NoFailure, FailureTypes...>;
};

// Define a FailureDetectorTag by taking the failures in a tuple
template<typename T>
struct FailureDetectorByTupleTag;

template<typename ...FailureTypes>
struct FailureDetectorByTupleTag<std::tuple<FailureTypes...>> : FailureDetectorTag<FailureTypes...> {};

template<typename Failure>
bool holds_failure(Failure&& failure)
{
    return not holds_alternative<NoFailure>(failure);
}

}