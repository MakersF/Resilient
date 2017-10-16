#pragma once

#include <type_traits>

#include <resilient/common/failable.hpp>
#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/execution_context.hpp>

namespace resilient {

struct ErrorReturn {};

template<typename T>
class Returns : public FailureDetectorTag<ErrorReturn>
{
public:
    Returns(T&& faliureValue)
    : d_failureValue(std::forward<T>(faliureValue))
    { }

    NoState preRun() { return NoState(); }

    template<typename Q>
    void postRun(NoState, const OperationResult<Q>& result, FailureSignal<failure_types>& fs)
    {
        if(not result.isException() and d_failureValue == result.getResult())
        {
            fs.signalFailure(ErrorReturn());
        }
    }

private:
    T d_failureValue;
};

}