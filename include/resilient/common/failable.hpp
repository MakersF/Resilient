#pragma once

#include <utility>
#include <cassert>

#include <boost/variant.hpp>

#include <resilient/common/utilities.hpp>
#include <resilient/common/base_variant.hpp>

namespace resilient {

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

    bool isFailure() const { return this->template is<Failure>(); }

    bool isValue() const { return !isFailure(); }

    const Value& value() const &
    {
        return this->template get<Value>();
    }

    Value& value() &
    {
        return this->template get<Value>();
    }

    Value value() &&
    {
        return std::move(*this).template get<Value>();
    }

    const Failure& failure() const &
    {
        return this->template get<Failure>();
    }

    Failure& failure() &
    {
        return this->template get<Failure>();
    }

    Failure failure() &&
    {
        return std::move(*this).template get<Failure>();
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