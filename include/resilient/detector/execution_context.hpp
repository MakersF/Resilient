#pragma once

#include <exception>
#include <utility>
#include <tuple>

#include <resilient/common/base_variant.hpp>

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

template<typename T>
class OperationResult : public ICallResult<T>
{
private:
    using ConstRefType = const std::decay_t<T>&;
    using ConstPtrT = const std::decay_t<T>*;
    using Base = Variant<std::exception_ptr, ConstPtrT>;

    Base d_data;
    bool d_isExceptionConsumed = false;

public:
    OperationResult() = default;
    OperationResult(const std::exception_ptr& ptr) : d_data(ptr) {}
    OperationResult(ConstRefType ref) : d_data(&ref) {}

    bool isExceptionConsumed() { return d_isExceptionConsumed; }
    void consumeException() override { d_isExceptionConsumed = true; }

    bool isException() const override
    {
        return holds_alternative<std::exception_ptr>(d_data);
    }

    const std::exception_ptr& getException() const override
    {
        return get<std::exception_ptr>(d_data);
    }

    ConstRefType getResult() const override
    {
        return *get<ConstPtrT>(d_data);
    }
};

namespace detail
{

template<typename T>
struct IFailureSignalImpl
{
    virtual void signalFailure(T&&) = 0;

    virtual ~IFailureSignalImpl() {}
};

}

template<typename ...T>
struct IFailureSignal : detail::IFailureSignalImpl<T>...
{
    virtual ~IFailureSignal() {}
};

// derive from impl so that it doesn't recursively expand all the tuples
template<typename ...T>
struct IFailureSignal<std::tuple<T...>> : detail::IFailureSignalImpl<T>...
{
    virtual ~IFailureSignal() {}
};

}