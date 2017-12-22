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


template<typename ...Types>
using Variant = boost::variant<Types...>;


template<typename T>
struct is_variant : std::false_type {};
template<typename ...Types>
struct is_variant<Variant<Types...>> : std::true_type {};

template<typename Q>
using if_is_variant = std::enable_if_t<is_variant<std::decay_t<Q>>::value, void*>;

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
    return boost::apply_visitor(IsType<T>(), std::forward<Variant>(variant));
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
    return strict_get<T>(std::forward<Variant>(variant));
}

template<typename Visitor, typename Variant, if_is_variant<Variant> = nullptr>
decltype(auto) visit(Visitor&& visitor, Variant&& variant)
{
    // Boost does not support r-value visitors or variants in apply_visitor.
    // We wrap the visitor so that we do the forwarding.
    RvalueForwardingVisitor<Visitor, Variant> wrapper{visitor};
    return boost::apply_visitor(wrapper, variant);
}

}
}