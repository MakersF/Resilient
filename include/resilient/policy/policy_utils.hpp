#pragma once

#include <resilient/common/variant.hpp>
#include <resilient/common/failable.hpp>

namespace resilient {

template<typename Failure>
struct add_failure_type
{
    template<typename ...NewFailures>
    using type = Variant<Failure, NewFailures...>;
};

template<typename ...Failures>
struct add_failure_type<Variant<Failures...>>
{
    template<typename ...NewFailures>
    using type = Variant<Failures..., NewFailures...>;
};

template<typename Failure, typename ...NewFailures>
using add_failure_type_t = typename add_failure_type<Failure>::template type<NewFailures...>;

// Given a Failable defines a new Failable which has the same value_type and
// 1. A failure_type which is the concatenation of NewFailures to the variant if it is a variant
// 2. A failure_type which is a variant of the old failure_type and the NewFailures
template<typename Failable, typename ...NewFailures>
using add_failure_to_failable_t = resilient::Failable<
    add_failure_type_t<typename Failable::failure_type, NewFailures...>,
        typename Failable::value_type>;

template<typename T, typename ...NewFailures>
using add_failure_to_noref_t = add_failure_to_failable_t<std::remove_reference_t<T>, NewFailures...>;

template<typename F, typename ...Args>
using noforward_result_of_t = std::result_of_t<F&(Args&...)>;

template<typename F, typename ...Args>
using forward_result_of_t = std::result_of_t<F&&(Args&&...)>;
}