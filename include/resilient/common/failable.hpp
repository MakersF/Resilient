#pragma once

#include <utility>
#include <cassert>

#include <boost/variant.hpp>

#include <resilient/common/utilities.hpp>
#include <resilient/common/variant.hpp>

namespace resilient {

// Represent the return value of an operation which might fail
template<typename Failure, typename Value>
class Failable : private Variant<Failure, Value>
{
private:
    using Base = Variant<Failure, Value>;

    const Base& asVariant() const { return *this; }
    Base& asVariant() { return *this; }

    friend class variant_traits<Failable>;

public:
    using failure_type = Failure;
    using value_type = Value;

    using Base::Base;
    using Base::operator=;
};

// Specialize variant traits so that Failable can be used like a variant
template<typename Failure, typename Value>
struct variant_traits<Failable<Failure, Value>>
{
    template<typename T>
    static decltype(auto) get_data(T&& failable)
    {
        return variant_traits<typename Failable<Failure, Value>::Base>::get_data(
            move_if_rvalue<T>(failable.asVariant()));
    }
};

template<typename T>
struct is_failable : std::false_type {};
template<typename F, typename V>
struct is_failable<Failable<F, V>> : std::true_type {};

template<typename Q>
using if_is_failable = std::enable_if_t<is_failable<std::decay_t<Q>>::value, void*>;

template<typename Failure, typename Value>
bool holds_failure(const Failable<Failure, Value>& failable)
{
    return holds_alternative<Failure>(failable);
}

template<typename Failure, typename Value>
bool holds_value(const Failable<Failure, Value>& failable)
{
    return holds_alternative<Value>(failable);
}

template<typename Failable, if_is_failable<Failable> = nullptr>
decltype(auto) get_failure(Failable&& failable)
{
    return get<typename std::decay_t<Failable>::failure_type>(std::forward<Failable>(failable));
}

template<typename Failable, if_is_failable<Failable> = nullptr>
decltype(auto) get_value(Failable&& failable)
{
    return get<typename std::decay_t<Failable>::value_type>(std::forward<Failable>(failable));
}

}