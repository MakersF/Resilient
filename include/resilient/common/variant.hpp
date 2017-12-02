#pragma once

#include <cassert>

#include <utility>
#include <tuple>
#include <resilient/common/utilities.hpp>
#include <boost/variant.hpp>

namespace resilient {

namespace detail {

// Visitor used to determine if the current type in the variant is T
template<typename T>
class IsType
    : public boost::static_visitor<bool>
{
public:

    bool operator()(const T&) const
    {
        return true;
    }

    template<typename Other>
    bool operator()(const Other&) const
    {
        return false;
    }
};

template<typename Variant>
struct as_boost_variant
{
    using type = same_const_ref_as_t<Variant, typename std::decay_t<Variant>::ImplVariant>;
};

template<typename Variant>
using as_boost_variant_t = typename as_boost_variant<Variant>::type;

// Allow to access the Failable as if it was a varian, so that all the variant functions can be used
template<typename Variant>
as_boost_variant_t<Variant> get_boost_variant(Variant&& v)
{
    return move_if_rvalue<Variant>(v.d_data);
}

template<typename Q>
using if_is_default_constructible = std::enable_if_t<std::is_default_constructible<Q>::value>;

}

template<typename ...Types>
class Variant;// Forward declare so that we can define is_variant before the implementation


template<typename T>
struct is_variant : std::false_type {};
template<typename ...Types>
struct is_variant<Variant<Types...>> : std::true_type {};

template<typename Q>
using if_is_variant = std::enable_if_t<is_variant<std::decay_t<Q>>::value, void*>;

template<typename Q>
using if_is_not_variant = std::enable_if_t<not is_variant<std::decay_t<Q>>::value, void*>;

template<typename ...Types>
class Variant
{
// wrap boost variant, providing a more c++17 similar interface.
// TODO use std::variant if available
public:
    template<typename U = argpack_element_t<0, Types...>,
             typename = detail::if_is_default_constructible<U>>
    Variant() {} // Use a template parameter U so that the evaluation is postponed
                 // until instantiation

    template<typename Other, if_is_variant<Other> = nullptr>
    Variant(Other&& other)
    : d_data(move_if_rvalue<Other>(other.d_data)) {}

    template<typename Other, if_is_not_variant<Other> = nullptr>
    Variant(Other&& other)
    : d_data(std::forward<Other>(other)) {}

    template<typename Other, if_is_variant<Other> = nullptr>
    Variant& operator=(Other&& other)
    {
        d_data = move_if_rvalue<Other>(other.d_data);
        return *this;
    }

    template<typename Other, if_is_not_variant<Other> = nullptr>
    Variant& operator=(Other&& other)
    {
        d_data = std::forward<Other>(other);
        return *this;
    }

private:
    using ImplVariant = boost::variant<Types...>;
    ImplVariant d_data;

    template<typename ...OtherTypes>
    friend class Variant;

    template<typename Variant>
    friend struct detail::as_boost_variant;

    template<typename Variant>
    friend detail::as_boost_variant_t<Variant> detail::get_boost_variant(Variant&& f);

};

template<typename T, typename Variant, if_is_variant<Variant> = nullptr>
bool holds_alternative(Variant&& variant) // TODO make it fail to compile if the variant can not hold T
{
    return boost::apply_visitor(
        detail::IsType<T>(),
        detail::get_boost_variant(std::forward<Variant>(variant)));
}

template<typename T, typename Variant, if_is_variant<Variant> = nullptr>
auto get(Variant&& variant) -> same_const_ref_as_t<Variant, T>
{
    assert(holds_alternative<T>(variant));
    return move_if_rvalue<Variant>(
            boost::strict_get<T>(
                detail::get_boost_variant(std::forward<Variant>(variant))));
}

template<typename Variant>
struct variant_cat;

template<typename ...Types>
struct variant_cat<Variant<Types...>>
{
    template<typename ...NewTypes>
    using type = Variant<Types..., NewTypes...>;
};

template<typename Variant, typename ...NewTypes>
using variant_cat_t = typename variant_cat<Variant>::template type<NewTypes...>;

template<typename Variant>
struct is_any_variant : std::false_type {};

template<typename ...Types>
struct is_any_variant<Variant<Types...>> : std::true_type {};
}