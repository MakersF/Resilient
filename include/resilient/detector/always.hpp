#pragma once

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

struct AlwaysError {};

// Always detect failure
class Always : public FailureDetectorTag<AlwaysError>
{
public:
    NoState preRun()
    {
        return NoState();
    }

    template<typename T>
    returned_failure_t<failure_types> postRun(NoState, ICallResult<T>& result)
    {
        if(result.isException())
        {
            // The goal of this class is to alwaws return error, independently from what happens.
            // We need to always consume the exception because if no one consumes it we are going to get
            // an error.
            // Since there is no guarantee another detector will consume the exception this needs to do it.
            result.consumeException();
        }
        return AlwaysError();
    }
};

}