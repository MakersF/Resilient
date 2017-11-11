#pragma once

#include <tuple>
#include <resilient/common/variant.hpp>

namespace resilient {

struct NoFailure {};

// Base struct that the Detectors can derive from.
// It defines a `failure_types`, a tuple containing the types of failure a detector can return
// and `failure`, the variant which contains the possible failure value, or NoFailure if no failure is detected.
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