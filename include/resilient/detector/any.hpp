#pragma once

#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <utility>

#include <resilient/detail/unique_types_tuple.hpp>
#include <resilient/detail/utilities.hpp>
#include <resilient/detail/variant_utils.hpp>
#include <resilient/detector/basedetector.hpp>

namespace resilient {

namespace detail {

template<typename Conds, size_t... I>
auto callPreRun(Conds& conditions, std::index_sequence<I...>)
{
    // Use initializer list to guarantee the order
    return std::tuple<decltype(std::get<I>(conditions).preRun())...>{
        std::get<I>(conditions).preRun()...};
}

// Call postRun on one of the detectors.
// Return an int so it's easy to use in the initializer_list
template<typename Failure, typename Detector, typename State, typename T>
int singlePostRun(Failure& mainFailure, Detector& condition, State&& state, ICallResult<T>& result)
{
    // The failure detection should happen in the same order as the failureconditions.
    // This means that we need to keep the failure of the first detector which triggers it.
    // So, if the Failure is already set we don't set it again otherwise we would override with
    // a later failure.
    decltype(auto) currentFailure{condition.postRun(std::forward<State>(state), result)};
    if (not holds_failure(mainFailure)) {
        assign_from_variant(mainFailure, std::move(currentFailure));
    }

    return 0;
}

template<typename Failure, typename... Detectors, typename... States, typename T, size_t... I>
Failure callPostRun(std::tuple<Detectors...>& conditions,
                    std::tuple<States...>&& state,
                    ICallResult<T>& result,
                    std::index_sequence<I...>)
{
    Failure mainFailure{NoFailure()};
    // Initializer list of comma expression
    (void) std::initializer_list<int>{singlePostRun(
        mainFailure, std::get<I>(conditions), std::move(std::get<I>(state)), result)...};

    return std::move(mainFailure);
}

} // namespace detail

// Check a set of detectors for any of them to fail
/**
 * @ingroup Detector
 * @brief Combine a group of detector, detecting a failure if any of them detect a failure.
 *
 * The order in which the detectors are invoked is the same as the order in which they are
 * added to the Any detector.
 *
 * `preRun()` and `postRun()` are always called on each detector, in the order in which they are
 * added.
 *
 * @tparam Detectors... The detectors used to check the failure.
 */
template<typename... Detectors>
class Any // Do not derive from the BaseDetectorTag as it's easier to define the type directly
{
public:
    /**
     * @brief Type required by the Detector concept.
     *
     * The `failure_types` are any of the possible failure types returned by the detectors composing
     * it.
     */
    // Concatenate all the failure from the detectors, removing duplicates.
    using failure_types = detail::unique_types_tuple_t<
        detail::tuple_flatten_t<typename std::decay_t<Detectors>::failure_types...>>;

    /**
     * @brief Define the type of the Any detector if we added a new detector to the current one.
     *
     * In other words, define what is going to be the type returned by `addDetector()`.
     *
     * @tparam Detector The new detector to add.
     */
    template<typename Detector>
    using after_adding_t = Any<Detectors..., Detector>;

    /**
     * @brief Construct an instance with a sequence of detectors.
     *
     * @param detectors The detectors to use.
     */
    explicit Any(std::tuple<Detectors...>&& detectors) : d_detectors(std::move(detectors)) {}

    /**
     * @brief Create a new Any detector with an additional detector added after all the currently
     * existing ones.
     *
     * @tparam Detector The type of the new detector to add.
     * @param detector The new detector to add.
     * @return A new instance of Any detector with the added detector.
     */
    template<typename Detector>
    after_adding_t<Detector> addDetector(Detector&& detector) &&
    {
        return after_adding_t<Detector>(std::tuple_cat(
            std::move(d_detectors), std::make_tuple(std::forward<Detector>(detector))));
    }

    /**
     * @brief Call `preRun*()` on all the detectors.
     *
     * @return A tuple containing the states of all the detectors.
     */
    auto preRun()
    {
        return detail::callPreRun(d_detectors, std::make_index_sequence<sizeof...(Detectors)>());
    }

    /**
     * @brief Detect failures using the detectors added to this class.
     *
     * @tparam States A tuple containing the state of all the detectors.
     * @tparam T The type returned by the detected function.
     * @param state See `States`.
     * @param result The result of invoking the detected function.
     * @return The Failure detected by the first detector, or NoFailure if no detectors detect a
     * failure.
     */
    template<typename... States, typename T>
    returned_failure_t<failure_types> postRun(std::tuple<States...>&& state, ICallResult<T>& result)
    {
        return detail::callPostRun<returned_failure_t<failure_types>>(
            d_detectors,
            std::move(state),
            result,
            std::make_index_sequence<sizeof...(Detectors)>());
    }

private:
    std::tuple<Detectors...> d_detectors;
};

/**
 * @brief Create a detector which uses the provided detectors in order to detect failures.
 * @related resilient::Any
 *
 * @tparam Detectors The types of the detectors to use.
 * @param detectors The detectors to use.
 * @return A detector which combines the provided detectors.
 */
template<typename... Detectors>
Any<Detectors...> anyOf(Detectors&&... detectors)
{
    return Any<Detectors...>(std::tuple<Detectors...>(std::forward<Detectors>(detectors)...));
}

} // namespace resilient