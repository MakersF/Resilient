#pragma once

#include <resilient/task/failable.hpp>

namespace resilient {

/**
 * @brief Create a `Failable` with a failure type.
 * @related resilient::Failable
 *
 * @tparam Failable The type of the `Failable`.
 * @param failure The failure to use.
 * @return Failable The `Failable` initialized with the failure.
 */
template<typename Failable, typename Failure>
Failable from_failure(Failure&& failure)
{
    return Failable{typename Failable::failure_type{std::forward<Failure>(failure)}};
}

/**
 * @brief Construct a `Failable` from another `Failable` with the same `value_type` and a narrower `failure_type`.
 * @related resilient::Failable
 *
 * A `failure_type` is narrower than another one if the variant which represents it can hold less types than the second one.
 * For example in `using B = resilient::add_failure_to_failable_t<A, NewFailure>`, `A` has a narrower `failure_type` than `B`
 * since `A` can not represent the failure `NewFailure`.
 *
 * @tparam Failable The type of the `Failable` to create.
 * @param failable The failable to use when creatint the new `Failable`.
 * @return Failable The `Failable` initialized from the narrower `failable`.
 */
template<typename Failable, typename OtherFailable>
Failable from_narrower_failable(OtherFailable&& failable)
{
    using FailureType = typename std::decay_t<Failable>::failure_type;
    if (holds_value(failable))
    {
        return Failable{get_value(std::forward<OtherFailable>(failable))};
    }
    else
    {
        return Failable{detail::construct_from_variant<FailureType>(get_failure(std::forward<OtherFailable>(failable)))};
    }
}

}