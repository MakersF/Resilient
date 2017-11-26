#pragma once

#include <exception>
#include <utility>
#include <tuple>
#include <resilient/common/invoke.hpp>

namespace resilient {

struct NoState {};

template<typename T>
class ICallResult
{
public:
    using ConstRefType = const std::decay_t<T>&;

    virtual void consumeException() = 0;
    virtual bool isException() const = 0;
    virtual const std::exception_ptr& getException() const = 0;
    virtual ConstRefType getResult() const = 0;

    virtual ~ICallResult() {}
};

template<typename Visitor, typename T>
constexpr decltype(auto) visit(Visitor&& visitor, ICallResult<T>& callresult)
{
    if (callresult.isException())
    {
        return detail::invoke(std::forward<Visitor>(visitor), callresult.getException());
    }
    else
    {
        return detail::invoke(std::forward<Visitor>(visitor), callresult.getResult());
    }
}

}