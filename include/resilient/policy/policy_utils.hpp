#pragma once

#include <resilient/common/variant.hpp>
#include <resilient/detail/invoke.hpp>
#include <resilient/task/failable.hpp>

namespace resilient {

namespace detail {

template<typename Failure>
struct add_failure_type
{
    template<typename... NewFailures>
    using type = Variant<Failure, NewFailures...>;
};

template<typename... Failures>
struct add_failure_type<Variant<Failures...>>
{
    template<typename... NewFailures>
    using type = Variant<Failures..., NewFailures...>;
};

} // namespace detail

/**
 * @brief Given a `Failure`, add new failures to the type.
 *
 * If the `Failure` is a `Variant` of failures then the new failures are appended to the `Variant`.
 * Otherwise define a `Varint` with the previous failure and the new failures.
 *
 * @tparam Failure The `Failure` to extend.
 * @tparam NewFailures The new `Failure`s to add.
 */
template<typename Failure, typename... NewFailures>
using add_failure_type_t =
    typename detail::add_failure_type<Failure>::template type<NewFailures...>;

/**
 * @brief Extend a `Failable`'s `Failure` with a new type.
 *
 * Given a `Failable` defines a new Failable which has the same value_type and
 * 1. A failure_type which is the concatenation of NewFailures to the variant if it is a variant.
 * 2. A failure_type which is a variant of the old failure_type and the NewFailures.
 *
 * @tparam Failable The `Failable` whose `failure_type` needs to be extended.
 * @tparam NewFailures The new `Failure`s to add to the `Failable`.
 */
template<typename Failable, typename... NewFailures>
using add_failure_to_failable_t =
    resilient::Failable<typename Failable::value_type,
                        add_failure_type_t<typename Failable::failure_type, NewFailures...>>;

/**
 * @brief Like `add_failure_to_failable_t`, but remove reference from the `Failable`.
 */
template<typename Failable, typename... NewFailures>
using add_failure_to_noref_failable_t =
    add_failure_to_failable_t<std::remove_reference_t<Failable>, NewFailures...>;

/**
 * @brief Define the type returned by invoking `F` with `Args...` both as lvalues.
 */
template<typename F, typename... Args>
using noforward_result_of_t = detail::invoke_result_t<F&, Args&...>;

/**
 * @brief Define the type returned by invoking `F` with `Args...` both forwarded.
 */
template<typename F, typename... Args>
using forward_result_of_t = detail::invoke_result_t<F, Args...>;
} // namespace resilient