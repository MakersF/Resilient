#pragma once

#include <exception>
#include <utility>
#include <tuple>

#include <resilient/common/base_variant.hpp>

namespace resilient {

struct NoState {};

template<typename T>
class OperationResult
{
private:
    using ConstRefT = const std::decay_t<T>&;
    using ConstPtrT = const std::decay_t<T>*;
    using Base = Variant<std::exception_ptr, ConstPtrT>;

    Base d_data;
public:
    OperationResult() = default;
    OperationResult(const std::exception_ptr& ptr) : d_data(ptr) {}
    OperationResult(ConstRefT ref) : d_data(&ref) {}

    bool isException() const
    {
        return holds_alternative<std::exception_ptr>(d_data);
    }

    ConstRefT getResult() const
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