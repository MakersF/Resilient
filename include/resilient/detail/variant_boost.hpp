#pragma once

#include <cassert>
#include <utility>
#include <tuple>

#include <boost/variant.hpp>
#include <boost/version.hpp>

#include <resilient/detail/utilities.hpp>

// Before version 1.65 boost does not support rvalues in strict_get
#define BOOST_STRICT_GET_NO_RVAL_SUPPORT BOOST_VERSION < 106500

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
    return move_if_not_lvalue<Variant>(v.d_data);
}

template<typename Q>
using if_is_default_constructible = std::enable_if_t<std::is_default_constructible<Q>::value>;


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
public:
    template<typename U = argpack_element_t<0, Types...>,
             typename = if_is_default_constructible<U>>
    Variant() {} // Use a template parameter U so that the evaluation is postponed
                 // until instantiation

    template<typename Other, if_is_variant<Other> = nullptr>
    Variant(Other&& other)
    : d_data(move_if_not_lvalue<Other>(other.d_data)) {}

    template<typename Other, if_is_not_variant<Other> = nullptr>
    Variant(Other&& other)
    : d_data(std::forward<Other>(other)) {}

    template<typename Other, if_is_variant<Other> = nullptr>
    Variant& operator=(Other&& other)
    {
        d_data = move_if_not_lvalue<Other>(other.d_data);
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
    friend struct as_boost_variant;

    template<typename Variant>
    friend as_boost_variant_t<Variant> get_boost_variant(Variant&& f);

};

// Visitor which forwards the value based on the ref-ness of the owning variant
template<typename Visitor, typename Variant>
struct RvalueForwardingVisitor
{
    // TODO implement so that the visitor does not have to define result_type
    using result_type = typename std::decay_t<Visitor>::result_type;

    Visitor& d_visitor;

    template<typename T>
    result_type operator()(T& value)
    {
        return detail::invoke(std::forward<Visitor>(d_visitor), move_if_not_lvalue<Variant>(value));
    }
};

template<typename T, typename Variant, if_is_variant<Variant> = nullptr>
bool holds_alternative(Variant&& variant) // TODO make it fail to compile if the variant can not hold T
{
    return boost::apply_visitor(IsType<T>(), get_boost_variant(std::forward<Variant>(variant)));
}

#if BOOST_STRICT_GET_NO_RVAL_SUPPORT
// We define the overload for rvals ourself
template<typename T, typename ...Types>
T&& strict_get(boost::variant<Types...>&& variant)
{
    return std::move(boost::strict_get<T>(variant));
}
#endif

template<typename T, typename Variant, if_is_variant<Variant> = nullptr>
auto get(Variant&& variant) -> same_const_ref_as_t<Variant, T>
{
    assert(holds_alternative<T>(variant));
    // Put boost::strict_get in the scope and use the unscoped call so that the overload we defined can be found
    using boost::strict_get;
    using resilient::detail::strict_get;
    return strict_get<T>(get_boost_variant(std::forward<Variant>(variant)));
}

template<typename Visitor, typename Variant, if_is_variant<Variant> = nullptr>
decltype(auto) visit(Visitor&& visitor, Variant&& variant)
{
    // Boost does not support r-value visitors or variants in apply_visitor.
    // We wrap the visitor so that we do the forwarding.
    RvalueForwardingVisitor<Visitor, Variant> wrapper{visitor};
    return boost::apply_visitor(wrapper, get_boost_variant(variant));
}

}
}