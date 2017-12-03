#pragma once

#include <tuple>
#include <resilient/common/variant.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

/**
 * @brief Type returned by the detectors when no failure is detected.
 */
struct NoFailure {};

/**
 * @brief Type representing that the detector does not need any state.
 */
struct NoState {};

/**
 * @ingroup Detector
 * @brief An helper base detector to simplify writing stateless detectors.
 *
 * It uses the CRT pattern to define the Detector concept, calling the derived Detector `detect()` method
 * to detect failures.
 *
 * The detectors which derive from the `StatelessDetector` need only to define a `detect()` function
 * which takes the `ICallResult`.
 *
 * @section howtouseit How to use it
 *
 * ```
 * struct MyStatelessDetector : FailureDetectorTag<MyFailure>, StatelessDetector<MyStatelessDetector>
 * {
 *      auto detect(ICallResult<T>& result)
 *      {
 *          ...
 *      }
 * }
 * ```
 *
 * @tparam Detector The type of the detector
 */
template<typename Detector>
struct StatelessDetector
{
    /**
     * @brief Does nothing.
     *
     * @return NoState.
     */
    NoState preRun()
    {
        return NoState();
    }

    /**
     * @brief Detect failures by invoking the `Detector`.
     *
     * @tparam T The type returned by the detected function.
     * @param NoState No state
     * @param result The result of invoking the detected function.
     * @return The result of invoking `detect()` on the `Detector`
     */
    template<typename T>
    decltype(auto) postRun(NoState, ICallResult<T>& result)
    {
        return static_cast<Detector*>(this)->detect(result);
    }


    /**
     * @see `postRun()`.
     */
    template<typename T>
    decltype(auto) postRun(NoState, ICallResult<T>& result) const
    {
        return static_cast<Detector*>(this)->detect(result);
    }

};

// Base struct that the Detectors can derive from.
// It defines a `failure_types`, a tuple containing the types of failure a detector can return
// and `failure`, the variant which contains the possible failure value, or NoFailure if no failure is detected.
/**
 * @ingroup Detector
 * @brief Helper struct which defines the required types for the detector concept.
 *
 * @tparam FailureTypes... The types of failures the deriving detector can detect.
 */
template<typename ...FailureTypes>
struct FailureDetectorTag
{
    /**
     * @brief Tuple composed of the possible failure types the detector can detect.
     *
     * Satisfy the required type definition for the Detector concept.
     */
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

/**
 * @ingroup Detector
 * @brief Type alias to the Variant containing either `NoFailure` or the failure types.
 *
 * @tparam FailureTypes... The failure types that can be returned.
 *      If it's a single tuple then the content of the tuple is used instead.
 */
template<typename ...FailureTypes>
using returned_failure_t = typename detail::returned_failure<FailureTypes...>::type;


// Define a FailureDetectorTag by taking the failures in a tuple
template<typename T>
struct FailureDetectorByTupleTag;

template<typename ...FailureTypes>
struct FailureDetectorByTupleTag<std::tuple<FailureTypes...>> : FailureDetectorTag<FailureTypes...> {};

/**
 * @ingroup Detector
 * @brief Whether a failure contains a failure or not.
 *
 * @tparam Failure The type of the Failure returned by the detector.
 * @tparam nullptr SFINAE out if it's not a Variant.
 * @param failure The Failure returned by the detector.
 * @return true A failure was detected.
 * @return false A failure was not detected.
 */
template<typename Failure, if_is_variant<Failure> = nullptr>
bool holds_failure(Failure&& failure)
{
    return not holds_alternative<NoFailure>(failure);
}

}