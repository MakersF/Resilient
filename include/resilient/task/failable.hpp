#pragma once

#include <utility>

#include <resilient/common/variant.hpp>
#include <resilient/detail/constref_util.hpp>
#include <resilient/detail/invoke.hpp>
#include <resilient/detail/utilities.hpp>

namespace resilient {

namespace detail {

template<typename Failable>
struct as_variant
{
    // The resulting type of forwarding the Failable::Base
    using type = decltype(move_if_not_lvalue<Failable>(
        std::declval<
            same_const_as_t<Failable, typename std::remove_reference_t<Failable>::Base>>()));
};

template<typename Failable>
using as_variant_t = typename as_variant<Failable>::type;

// Allow to access the Failable as if it was a varian, so that all the variant functions can be used.
// Note: the return must always be a reference otherwise we copy the variant, and boost fails as it can't
//       create a variant from a Failable.
//       std::variant works, not sure why (probably because get_variant is friend and can see that Failable
//       derives from variant)
template<typename Failable>
as_variant_t<Failable> get_variant(Failable&& f)
{
    return move_if_not_lvalue<Failable>(f);
}

template<typename Failable>
using failable_value_type_t = typename std::remove_reference_t<Failable>::value_type;

template<typename Failable>
using failable_failure_type_t = typename std::remove_reference_t<Failable>::failure_type;

template<typename Failable>
using get_value_return_type = decltype(get_value(std::declval<Failable>()));

template<typename Failable>
using get_failure_return_type = decltype(get_failure(std::declval<Failable>()));

template<typename Failable, typename InvocableResult>
using if_is_convertible_to_get_value_return_type_of =
    std::enable_if_t<std::is_convertible<InvocableResult, get_value_return_type<Failable>>::value,
                     void*>;

} // namespace detail

template<typename Value, typename Failure>
class Failable; // Forward declare so that we can define is_failable before the implementation

template<typename T>
struct is_failable : std::false_type
{
};
template<typename V, typename F>
struct is_failable<Failable<V, F>> : std::true_type
{
};

template<typename Q>
using if_is_failable = std::enable_if_t<is_failable<std::decay_t<Q>>::value, void*>;

template<typename Q>
using if_is_not_failable = std::enable_if_t<not is_failable<std::decay_t<Q>>::value, void*>;

/**
 * @ingroup Task
 * @brief A class to represent the result of an operation which might fail.
 *
 * A `Failable` is either a value or a failure.
 * The value is the return value of the invoked function, the failure is an object representing the
 * failure which happened.
 *
 * @tparam Value The value.
 * @tparam Failure The type of the failure.
 */
template<typename Value, typename Failure>
class Failable : private Variant<Value, Failure>
{
private:
    using Base = Variant<Value, Failure>;

    template<typename Failable>
    friend struct detail::as_variant;

    template<typename Failable_>
    friend detail::as_variant_t<Failable_> detail::get_variant(Failable_&& f);
    // Make the get_variant friend so that it knows we derive from Variant

public:
    /**
     * @brief Alias for the type of the failure.
     */
    using failure_type = Failure;
    /**
     * @brief Alias for the type of the value.
     */
    using value_type = Value;

    /**
     * @brief Create a `Failable` from another `Failable`.
     *
     * @param other The other `Failable`.
     */
    template<typename Other, if_is_failable<Other> = nullptr>
    Failable(Other&& other) : Base(detail::get_variant(std::forward<Other>(other)))
    {
    }

    /**
     * @brief Create a `Failable` from a either a `Failure` or a `Value`.
     *
     * @param other The object to instantiate the `Failable` with.
     */
    template<typename Other, if_is_not_failable<Other> = nullptr>
    Failable(Other&& other) : Base(std::forward<Other>(other))
    {
    }

    /**
     * @brief Assign the value of another `Failable` to this `Failable`.
     *
     * @param other The other `Failable`.
     * @return This `Failable`.
     */
    template<typename Other, if_is_failable<Other> = nullptr>
    Failable& operator=(Other&& other)
    {
        detail::get_variant(*this) = detail::get_variant(std::forward<Other>(other));
        return *this;
    }

    /**
     * @brief Assign an object to this `Failable`.
     *
     * @param other The object to assign.
     * @return This `Failable`.
     */
    template<typename Other, if_is_not_failable<Other> = nullptr>
    Failable& operator=(Other&& other)
    {
        detail::get_variant(*this) = std::forward<Other>(other);
        return *this;
    }
};

/**
 * @brief Check whether the `Failable` contains a failure.
 * @related resilient::Failable
 *
 * @param failable The `Failable` to check.
 * @return true If the `Failable` contains the failure.
 * @return false If the `Failable` contains the value.
 */
template<typename Value, typename Failure>
bool holds_failure(const Failable<Value, Failure>& failable)
{
    return holds_alternative<Failure>(detail::get_variant(failable));
}

/**
 * @brief Check whether the `Failable` contains a value.
 * @related resilient::Failable
 *
 * @param failable The `Failable` to check.
 * @return true If the `Failable` contains the value.
 * @return false If the `Failable` contains the failure.
 */
template<typename Value, typename Failure>
bool holds_value(const Failable<Value, Failure>& failable)
{
    return holds_alternative<Value>(detail::get_variant(failable));
}

/**
 * @brief Get the failure in the `Failable`.
 * @related resilient::Failable
 *
 * @pre The `Failable` is currently holding a failure.
 *
 * @param failable The `Failable` to get the failure from.
 * @return The failure.
 */
template<typename Failable, if_is_failable<Failable> = nullptr>
auto get_failure(Failable&& failable)
    -> detail::same_const_ref_as_t<Failable, detail::failable_failure_type_t<Failable>>
{
    return get<detail::failable_failure_type_t<Failable>>(
        detail::get_variant(std::forward<Failable>(failable)));
}

/**
 * @brief Get the value in the `Failable`.
 * @related resilient::Failable
 *
 * @pre The `Failable` is currently holding a value.
 *
 * @param failable The `Failable` to get the value from.
 * @return The value.
 */
template<typename Failable, if_is_failable<Failable> = nullptr>
auto get_value(Failable&& failable)
    -> detail::same_const_ref_as_t<Failable, detail::failable_value_type_t<Failable>>
{
    return get<detail::failable_value_type_t<Failable>>(
        detail::get_variant(std::forward<Failable>(failable)));
}

/**
 * @brief Get the value in the `Failable` or default_value if not present.
 * @related resilient::Failable
 *
 * @param failable The `Failable` to get the value from if present.
 * @param default_value The default value if a value is not present in `Failable`
 * @return The value in `Failable` if present, default_value otherwise
 */
template<typename Failable, if_is_failable<Failable> = nullptr>
auto get_value_or(Failable&& failable, detail::get_value_return_type<Failable> default_value)
    -> detail::get_value_return_type<Failable>
{
    if (holds_failure(failable)) {
        return std::forward<decltype(default_value)>(default_value);
    }
    return get_value(std::forward<Failable>(failable));
}

/**
 * @brief Get the combination of two `Failable`s sharing the same value_type.
 * @related resilient::Failable
 *
 * Get the value combining two `Failable`s.
 * The value of the first `Failable` is preferred.
 * The failure of the second `Failable` is preferred.
 *
 * @param failable A failable
 * @param other A failable with a compatible value_type of `failable`.
 * @return A `Failable`.
 *         It holds the value of `failable` if it contains a value.
 *         It holds the value of `other` if it contains a value.
 *         It holds the failure of `other` if both failables hold a failure.
 *         The returned type is always a value (not a reference).
 */
template<typename Failable,
         typename OtherFailable,
         if_is_failable<Failable> = nullptr,
         if_is_failable<OtherFailable> = nullptr>
auto get_value_or(Failable&& failable, OtherFailable&& other)
    -> std::remove_const_t<std::remove_reference_t<OtherFailable>>
{
    // We need to always return a value. The reason is that if Failable holds a value then we
    // need to return an instance of OtherFailable which holds the value.
    // This means that we need to create it on the stack, and returning a reference would lead to
    // dangling reference.
    if (holds_failure(failable)) {
        return std::forward<OtherFailable>(other);
    }
    return get_value(std::forward<Failable>(failable));
}

/**
 * @brief Get the value in failable if it holds a value, the returned value of invoking `invocable` otherwise.
 * @related resilient::Failable
 * @see get_value_or()
 *
 * The function is similar to `get_value_or()` but allows to compute the default value lazily.
 *
 * @param failable The `Failable` to get the value from if present.
 * @param invocable The object to call if failable does not hold a value.
 * @return The value stored in failable or the value returned by invocable.
 */
template<typename Failable,
         typename Invocable,
         typename InvocableResult = detail::invoke_result_t<Invocable>,
         if_is_failable<Failable> = nullptr,
         detail::if_is_convertible_to_get_value_return_type_of<Failable, InvocableResult> = nullptr>
auto get_value_or_invoke(Failable&& failable, Invocable&& invocable)
    -> detail::get_value_return_type<Failable>
{
    if (holds_failure(failable)) {
        return std::forward<Invocable>(invocable)();
    }
    return get_value(std::forward<Failable>(failable));
}

/**
 * @brief Get the combination of `failable` and the result of invoking `invocable`.
 * @related resilient::Failable
 * @see get_value_or
 *
 * Equivalent to `get_value_or()` with a `Failable` which is provided by lazily calling
 * `invocable`.
 * `invocable` is only called if `failable` does not contain a value.
 *
 * @param failable A `Failable`.
 * @param invocable A function which returns a `Failable` with value_type compatible to
 *                  the one of `failable`.
 * @return The combination of `failable` and the result of `invocable`.
 *         See `get_value_or()` for what the result contains.
 */
template<typename Failable,
         typename Invocable,
         typename InvocableResult = detail::invoke_result_t<Invocable>,
         if_is_failable<Failable> = nullptr,
         if_is_failable<InvocableResult> = nullptr>
auto get_value_or_invoke(Failable&& failable, Invocable&& invocable)
    -> std::remove_const_t<std::remove_reference_t<InvocableResult>>
{
    if (holds_failure(failable)) {
        return std::forward<Invocable>(invocable)();
    }
    return get_value(std::forward<Failable>(failable));
}

/**
 * @brief Apply a visitor to the `Failable`.
 * @related resilient::Failable
 *
 * Invoke the visitor with either the failure or the value, depending on what
 * the `Failable` is currently holding.
 *
 * @param visitor The visitor to use.
 * @param failable The `Failable` to visit.
 * @return The value returned by the visitor.
 */
template<typename Visitor, typename Failable, if_is_failable<Failable> = nullptr>
decltype(auto) visit(Visitor&& visitor, Failable&& failable)
{
    if (holds_failure(failable)) {
        return detail::invoke(std::forward<Visitor>(visitor),
                              get_failure(std::forward<Failable>(failable)));
    }
    else
    {
        return detail::invoke(std::forward<Visitor>(visitor),
                              get_value(std::forward<Failable>(failable)));
    }
}

} // namespace resilient