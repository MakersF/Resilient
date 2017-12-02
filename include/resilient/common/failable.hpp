#pragma once

#include <utility>

#include <resilient/common/utilities.hpp>
#include <resilient/common/variant.hpp>

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
    return move_if_rvalue<Failable>(f);
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

// Represent the return value of an operation which might fail
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
    using failure_type = Failure;
    using value_type = Value;

    template<typename Other, if_is_failable<Other> = nullptr>
    Failable(Other&& other) : Base(detail::get_variant(std::forward<Other>(other)))
    {}

    template<typename Other, if_is_not_failable<Other> = nullptr>
    Failable(Other&& other) : Base(std::forward<Other>(other))
    {}

    template<typename Other, if_is_failable<Other> = nullptr>
    Failable& operator=(Other&& other)
    {
        detail::get_variant(*this) = detail::get_variant(std::forward<Other>(other));
        return *this;
    }

    template<typename Other, if_is_not_failable<Other> = nullptr>
    Failable& operator=(Other&& other)
    {
        detail::get_variant(*this) = std::forward<Other>(other);
        return *this;
    }
};

template<typename Failure, typename Value>
bool holds_failure(const Failable<Failure, Value>& failable)
{
    return holds_alternative<Failure>(detail::get_variant(failable));
}

template<typename Failure, typename Value>
bool holds_value(const Failable<Failure, Value>& failable)
{
    return holds_alternative<Value>(detail::get_variant(failable));
}

template<typename Failable, if_is_failable<Failable> = nullptr>
decltype(auto) get_failure(Failable&& failable)
{
    return get<typename std::decay_t<Failable>::failure_type>(
        detail::get_variant(std::forward<Failable>(failable)));
}

template<typename Failable, if_is_failable<Failable> = nullptr>
decltype(auto) get_value(Failable&& failable)
{
    return get<typename std::decay_t<Failable>::value_type>(
        detail::get_variant(std::forward<Failable>(failable)));
}
}

}