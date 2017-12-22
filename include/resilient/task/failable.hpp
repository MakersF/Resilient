#pragma once

#include <utility>

#include <resilient/detail/utilities.hpp>
#include <resilient/common/variant.hpp>
#include <resilient/detail/invoke.hpp>

namespace resilient {

namespace detail {

template<typename Failable>
struct as_variant
{
    using type = same_const_ref_as_t<Failable, typename std::decay_t<Failable>::Base>;
};

template<typename Failable>
using as_variant_t = typename as_variant<Failable>::type;

// Allow to access the Failable as if it was a varian, so that all the variant functions can be used
template<typename Failable>
as_variant_t<Failable> get_variant(Failable&& f)
{
    return move_if_not_lvalue<Failable>(f);
}

}

template<typename Failure, typename Value>
class Failable; // Forward declare so that we can define is_failable before the implementation

template<typename T>
struct is_failable : std::false_type {};
template<typename F, typename V>
struct is_failable<Failable<F, V>> : std::true_type {};

template<typename Q>
using if_is_failable = std::enable_if_t<is_failable<std::decay_t<Q>>::value, void*>;

template<typename Q>
using if_is_not_failable = std::enable_if_t<not is_failable<std::decay_t<Q>>::value, void*>;

/**
 * @ingroup Task
 * @brief A class to represent the result of an operation which might fail.
 *
 * A `Failable` is either a value or a failure.
 * The value is the return value of the invoked function, the failure is an object representing the failure which happened.
 *
 * @tparam Failure The type of the failure.
 * @tparam Value The value.
 */
template<typename Failure, typename Value>
class Failable : private Variant<Failure, Value>
{
private:
    using Base = Variant<Failure, Value>;

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
    {}

    /**
     * @brief Create a `Failable` from a either a `Failure` or a `Value`.
     *
     * @param other The object to instantiate the `Failable` with.
     */
    template<typename Other, if_is_not_failable<Other> = nullptr>
    Failable(Other&& other) : Base(std::forward<Other>(other))
    {}

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
template<typename Failure, typename Value>
bool holds_failure(const Failable<Failure, Value>& failable)
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
template<typename Failure, typename Value>
bool holds_value(const Failable<Failure, Value>& failable)
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
decltype(auto) get_failure(Failable&& failable)
{
    return get<typename std::decay_t<Failable>::failure_type>(
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
decltype(auto) get_value(Failable&& failable)
{
    return get<typename std::decay_t<Failable>::value_type>(
        detail::get_variant(std::forward<Failable>(failable)));
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
    if(holds_failure(failable))
    {
        return detail::invoke(std::forward<Visitor>(visitor), get_failure(std::forward<Failable>(failable)));
    }
    else
    {
        return detail::invoke(std::forward<Visitor>(visitor), get_value(std::forward<Failable>(failable)));
    }
}

}