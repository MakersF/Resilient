#pragma once

#include <tuple>
#include <resilient/common/variant.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

struct NoFailure {};

struct NoState {};

template<typename Detector>
struct StatelessDetector
{
    NoState preRun()
    {
        return NoState();
    }

    template<typename T>
    decltype(auto) postRun(NoState, ICallResult<T>& result)
    {
        return static_cast<Detector*>(this)->detect(result);
    }


    template<typename T>
    decltype(auto) postRun(NoState, ICallResult<T>& result) const
    {
        return static_cast<Detector*>(this)->detect(result);
    }

};
// Base struct that the Detectors can derive from.
// It defines a `failure_types`, a tuple containing the types of failure a detector can return
// and `failure`, the variant which contains the possible failure value, or NoFailure if no failure is detected.
template<typename ...FailureTypes>
struct FailureDetectorTag
{
    using failure_types = std::tuple<FailureTypes...>;
};

namespace detail {

template<typename ...FailureTypes>
struct returned_failure
{
    using type = Variant<NoFailure, FailureTypes...>;
};

template<typename ...FailureTypes>
struct returned_failure<std::tuple<FailureTypes...>>
{
    using type = Variant<NoFailure, FailureTypes...>;
};

}
template<typename ...FailureTypes>
using returned_failure_t = typename detail::returned_failure<FailureTypes...>::type;


// Define a FailureDetectorTag by taking the failures in a tuple
template<typename T>
struct FailureDetectorByTupleTag;

template<typename ...FailureTypes>
struct FailureDetectorByTupleTag<std::tuple<FailureTypes...>> : FailureDetectorTag<FailureTypes...> {};

template<typename Failure, if_is_variant<Failure> = nullptr>
bool holds_failure(Failure&& failure)
{
    return not holds_alternative<NoFailure>(failure);
}

}