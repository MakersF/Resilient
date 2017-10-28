#pragma once

#include <exception>
#include <utility>
#include <tuple>

#include <resilient/common/base_variant.hpp>

namespace resilient {

struct NoState {};

template<typename T>
class OperationResult
: private BaseVariant<OperationResult<T>, std::exception_ptr, const std::decay_t<T>*>
{
private:
    using ConstRefT = const std::decay_t<T>&;
    using ConstPtrT = const std::decay_t<T>*;
    using Base = BaseVariant<OperationResult<T>, std::exception_ptr, ConstPtrT>;
public:
    OperationResult() = default;
    OperationResult(const std::exception_ptr& ptr) : Base(ptr) {}
    OperationResult(ConstRefT ref) : Base(&ref) {}

    OperationResult& operator=(const std::exception_ptr& ptr)
    {
        (*static_cast<Base*>(this)) = ptr;
        return *this;
    }

    OperationResult& operator=(ConstRefT ref)
    {
        (*static_cast<Base*>(this)) = &ref;
        return *this;
    }

    bool isException() const { return this->template is<std::exception_ptr>(); }
    ConstRefT getResult() const { return *this->template get<ConstPtrT>(); }

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