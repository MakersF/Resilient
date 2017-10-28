#pragma once

#include <cassert>

#include <utility>
#include <tuple>
#include <resilient/common/utilities.hpp>
#include <boost/variant.hpp>

namespace resilient {

namespace detail {

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

template<typename Q>
using if_is_default_constructible = std::enable_if_t<std::is_default_constructible<Q>::value>;

}

template<typename Variant>
struct variant_traits; // Specialize it for your variant type

template<typename Other>
using variant_traits_t = variant_traits<std::decay_t<Other>>;

template<typename Q>
using is_variant = is_complete_type<variant_traits_t<Q>>;
// A type is a variant if a specialization of variant_traits exists for it.
// If the specialization is defined than the type is complete, otherwise is
// not complete (since by default it's not defined but only declared).

template<typename Q>
using if_is_variant = std::enable_if_t<is_variant<Q>::value, void*>;

template<typename Q>
using if_is_not_variant = std::enable_if_t<not is_variant<Q>::value, void*>;

template<typename ...Types>
class Variant
{
public:
    template<typename U = argpack_element_t<0, Types...>,
             typename = detail::if_is_default_constructible<U>>
    Variant() {}

    template<typename Self, if_is_variant<Self> = nullptr>
    Variant(Self&& other)
    : d_data(variant_traits_t<Self>::get_data(other)) {}

    template<typename Other, if_is_not_variant<Other> = nullptr>
    Variant(Other&& other)
    : d_data(std::forward<Other>(other)) {}

    template<typename Self, if_is_variant<Self> = nullptr>
    Variant& operator=(Self&& other)
    {
        d_data = move_if_rvalue<Self>(variant_traits_t<Self>::get_data(other));
        return *this;
    }

    template<typename Other, if_is_not_variant<Other> = nullptr>
    Variant& operator=(Other&& other)
    {
        d_data = std::forward<Other>(other);
        return *this;
    }

private:
    boost::variant<Types...> d_data;

    template<typename ...OtherTypes>
    friend class Variant;

    friend class variant_traits<Variant>;
};

template<typename ...Types>
struct variant_traits<Variant<Types...>>
{
    template<typename V>
    static same_const_ref_as_t<V, boost::variant<Types...>> get_data(V&& variant)
    {
        return move_if_rvalue<V>(variant.d_data);
    }
};

template<typename T, typename Variant, if_is_variant<Variant> = nullptr>
bool holds_alternative(Variant&& variant)
{
    return boost::apply_visitor(
        detail::IsType<T>(),
        variant_traits_t<Variant>::get_data(variant));
}

template<typename T, typename Variant, if_is_variant<Variant> = nullptr>
auto get(Variant&& variant) -> same_const_ref_as_t<Variant, T>
{
    assert(holds_alternative<T>(variant));
    return move_if_rvalue<Variant>(
            boost::strict_get<T>(
                variant_traits_t<Variant>::get_data(
                    std::forward<Variant>(variant))));
}

template<typename Derived, typename ...Failures>
class BaseVariant
{
private:
    // if we derive publicly from boost::variant some weird overload resolution failures error come up
    // when trying to assign to it

    template<typename Class>
    using is_this_derived_t = std::is_same<std::decay_t<Class>, Derived>;

public:
    // Constructors

    // We define a template parameter U because if we used Derived
    // the template would be evaluated at the class instantiaion time,
    // and we would fail in case the first element is not default constructible
    template<typename U = argpack_element_t<0, Failures...>,
             typename = std::enable_if_t<std::is_default_constructible<U>::value>>
    BaseVariant() : d_data() { }

    template<typename OtherSelf,
             typename std::enable_if_t<is_this_derived_t<OtherSelf>::value, void*> = nullptr>
    BaseVariant(OtherSelf&& self)
    : d_data(move_if_rvalue<OtherSelf>(self.d_data))
    { }

    template<typename Other,
             typename std::enable_if_t<!is_this_derived_t<Other>::value, void*> = nullptr>
    BaseVariant(Other&& value)
    : d_data(std::forward<Other>(value))
    { }

    template<typename OtherSelf,
             typename std::enable_if_t<is_this_derived_t<OtherSelf>::value, void*> = nullptr>
    Derived& operator=(OtherSelf&& self)
    {
        d_data = move_if_rvalue<OtherSelf>(self.d_data);
        return *static_cast<Derived*>(this);
    }

    template<typename Other,
             typename std::enable_if_t<!is_this_derived_t<Other>::value, void*> = nullptr>
    Derived& operator=(Other&& value)
    {
        d_data = std::forward<Other>(value);
        return *static_cast<Derived*>(this);
    }

    template<typename T>
    bool is() const
    {
        return boost::apply_visitor(detail::IsType<T>(), this->d_data);
    }

    template<typename T>
    const T& get() const
    {
        assert(is<T>());
        return boost::strict_get<T>(this->d_data);
    }

    template<typename T>
    T& get()
    {
        assert(is<T>());
        return boost::strict_get<T>(this->d_data);
    }

    template<typename T>
    T&& get() &&
    {
        assert(is<T>());
        return std::move(boost::strict_get<T>(this->d_data));
    }

protected:
    boost::variant<Failures...> d_data;
};

}