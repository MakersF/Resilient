#pragma once

#include <utility>
#include <cassert>

#include <boost/variant.hpp>

#include <resilient/common/utilities.hpp>
#include <resilient/common/base_variant.hpp>

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

} // namespace detail

template<typename Failure, typename Value>
struct failable_tag
{
    using failure_type = Failure;
    using value_type = Value;
};

template<typename Failure, typename Value>
class Failable :
    private BaseVariant<Failable<Failure, Value>, Failure, Value>,
    public failable_tag<Failure, Value>
{
private:
    using Base = BaseVariant<Failable<Failure, Value>, Failure, Value>;
    friend Base;

public:
    using Base::Base;

    bool isFailure() const { return boost::apply_visitor(detail::IsType<Failure>(), this->d_data); }

    bool isValue() const { return !isFailure(); }

    const Value& value() const &
    {
        assert(isValue());
        return boost::strict_get<Value>(this->d_data);
    }

    Value& value() &
    {
        assert(isValue());
        return boost::strict_get<Value>(this->d_data);
    }

    Value value() &&
    {
        assert(isValue());
        return boost::strict_get<Value>(std::move(this->d_data));
    }
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
class Failure : private BaseVariant<Failure<Failures...>, Failures...>
{
private:
    using Base = BaseVariant<Failure<Failures...>, Failures...>;
    friend Base;

public:
    using Base::Base;
};

}