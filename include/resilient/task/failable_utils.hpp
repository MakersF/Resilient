#pragma once

#include <resilient/common/variant.hpp>
#include <resilient/detail/variant_utils.hpp>
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

namespace {

// Not all variants can be constructed from a narrower variant, so if the failure is a
// variant we use construct_from_variant to construct it.
template<typename FailureType, typename Variant, if_is_variant<Variant> = nullptr>
FailureType from_narrower_failure(Variant&& failure)
{
    return detail::construct_from_variant<FailureType>(std::forward<Variant>(failure));
}

// For non variant types we construct the FailureType with its constructor
template<typename FailureType, typename Failure, if_is_not_variant<Failure> = nullptr>
FailureType from_narrower_failure(Failure&& failure)
{
    return FailureType{std::forward<Failure>(failure)};
}

} // namespace

/**
 * @brief Construct a `Failable` from another `Failable` with the same `value_type` and a narrower
 * `failure_type`.
 * @related resilient::Failable
 *
 * A `failure_type` `A` is narrower than another type `B` if the variant which represents it can hold a
 * subset of the types `B` can hold. For example in `using B = resilient::add_failure_to_failable_t<A,
 * NewFailure>`, `A` has a narrower `failure_type` than `B` since `A` can not represent the failure
 * `NewFailure`.
 *
 * @tparam Failable The type of the `Failable` to create.
 * @param failable The failable to use when creatint the new `Failable`.
 * @return Failable The `Failable` initialized from the narrower `failable`.
 */
template<typename Failable, typename OtherFailable>
Failable from_narrower_failable(OtherFailable&& failable)
{
    using FailureType = typename std::remove_reference_t<Failable>::failure_type;
    if (holds_value(failable)) {
        return Failable{get_value(std::forward<OtherFailable>(failable))};
    }
    else
    {
        return Failable{
            from_narrower_failure<FailureType>(get_failure(std::forward<OtherFailable>(failable)))};
    }
}

} // namespace resilient