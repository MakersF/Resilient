#pragma once

#include <exception>
#include <utility>
#include <tuple>

#include <resilient/common/variant.hpp>

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

}