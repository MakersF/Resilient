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