#pragma once

#include <type_traits>

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

struct ErrorReturn {};

// Detects error if a function returns a specific value
template<typename T>
class Returns : public FailureDetectorTag<ErrorReturn>
{
public:
    Returns(T&& faliureValue)
    : d_failureValue(std::forward<T>(faliureValue))
    { }

    NoState preRun() { return NoState(); }

    template<typename Q>
    failure postRun(NoState, ICallResult<Q>& result)
    {
        if(not result.isException() and d_failureValue == result.getResult())
        {
            return ErrorReturn();
        }
        else
        {
            return NoFailure();
        }
    }

private:
    T d_failureValue;
};

}