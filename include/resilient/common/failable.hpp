#pragma once

#include <utility>
#include <cassert>

#include <boost/variant.hpp>

#include <resilient/common/utilities.hpp>

namespace resilient {

// TODO
// T -> F(T)
// F(T) -> T
// g(x) -> g(F(x))
// g(F(x)) -> g(x)

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

} // namespace detail

template<typename Failure, typename Value>
struct failable_tag
{
    static constexpr bool is_failable = true;

    using failure_type = Failure;
    using value_type = Value;
};

template<typename Failure, typename Value>
class Failable : public failable_tag<Failure, Value>
{
    // if we derive publicly from boost::variant some weird overload resolution failures error come up
    // when using gtest
private:
    template<typename Class>
    static constexpr bool is_this_failable = std::is_same<std::decay_t<Class>, Failable>::value;

public:
    template<typename U = Failure, // Trick required to SFINAE at method instantiation
             typename = std::enable_if_t<std::is_default_constructible<U>::value>>
    Failable() : d_data() { }

    // Constructors
    template<typename OtherFailable,
             typename std::enable_if_t<is_this_failable<OtherFailable>, void*> = nullptr>
    Failable(OtherFailable&& failable)
    : d_data(move_if_rvalue<OtherFailable>(failable.d_data))
    { }

    template<typename Other,
             typename std::enable_if_t<!is_this_failable<Other>, void*> = nullptr>
    Failable(Other&& value)
    : d_data(std::forward<Other>(value))
    { }

    template<typename OtherFailable,
             typename std::enable_if_t<is_this_failable<OtherFailable>, void*> = nullptr>
    Failable& operator=(OtherFailable&& failable)
    {
        d_data = move_if_rvalue<OtherFailable>(failable.d_data);
        return *this;
    }

    template<typename Other,
             typename std::enable_if_t<!is_this_failable<Other>, void*> = nullptr>
    Failable& operator=(Other&& value)
    {
        d_data = std::forward<Other>(value);
        return *this;
    }

    bool isFailure() const { return boost::apply_visitor(detail::IsType<Failure>(), d_data); }

    bool isValue() const { return !isFailure(); }

    const Value& value() const &
    {
        assert(isValue());
        return boost::strict_get<Value>(d_data);
    }

    Value& value() &
    {
        assert(isValue());
        return boost::strict_get<Value>(d_data);
    }

    Value value() &&
    {
        assert(isValue());
        return boost::strict_get<Value>(std::move(d_data));
    }

private:
    boost::variant<Failure, Value> d_data;
};

template<typename Failure, typename T>
Failable<Failure, T> make_failable(T&& value)
{
    return Failable<Failure, T>(std::forward<T>(value));
}

/// Create a failure given a failable type (needs default construction Failable)
template<typename Failable>
Failable failure_for()
{
    static_assert(Failable::is_failable, "The type must be a failable");
    return Failable();
}

template<typename Failure, typename T>
Failable<Failure, T> failure()
{
    return Failable<Failure, T>();
}

// Note: the template arguments are switched so that the user passes only the type of the value
template<typename T, typename Failure>
Failable<Failure, T> failure(Failure&& failure)
{
    return Failable<Failure, T>(std::forward<Failure>(failure));
}

template<typename ...Failures>
class Failure
{
private:
    template<typename Class>
    static constexpr bool is_this_failure = std::is_same<std::decay_t<Class>, Failure>::value;

public:
    // Constructors
    template<typename OtherFailure,
             typename std::enable_if_t<is_this_failure<OtherFailure>, void*> = nullptr>
    Failure(OtherFailure&& failure)
    : d_data(move_if_rvalue<OtherFailure>(failure.d_data))
    { }

    template<typename Other,
             typename std::enable_if_t<!is_this_failure<Other>, void*> = nullptr>
    Failure(Other&& value)
    : d_data(std::forward<Other>(value))
    { }

    template<typename OtherFailure,
             typename std::enable_if_t<is_this_failure<OtherFailure>, void*> = nullptr>
    Failure& operator=(OtherFailure&& failure)
    {
        d_data = move_if_rvalue<OtherFailure>(failure.d_data);
        return *this;
    }

    template<typename Other,
             typename std::enable_if_t<!is_this_failure<Other>, void*> = nullptr>
    Failure& operator=(Other&& value)
    {
        d_data = std::forward<Other>(value);
        return *this;
    }
private:
    boost::variant<Failures...> d_data;
};

}