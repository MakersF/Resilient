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

    bool isException() const { return this->template is<std::exception_ptr>(); }
    ConstRefT getResult() const { return *this->template get<ConstPtrT>(); }

};

namespace detail
{

template<typename T>
struct FailureSignalImpl
{
    virtual void signalFailure(T&&) = 0;

    virtual ~FailureSignalImpl() {}
};

}

template<typename ...T>
struct FailureSignal : detail::FailureSignalImpl<T>...
{
    virtual ~FailureSignal() {}
};

// derive from impl so that it doesn't recursively expand all the tuples
template<typename ...T>
struct FailureSignal<std::tuple<T...>> : detail::FailureSignalImpl<T>...
{
    virtual ~FailureSignal() {}
};

}