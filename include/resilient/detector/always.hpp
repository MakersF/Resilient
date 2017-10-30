#pragma once

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/execution_context.hpp>

namespace resilient {

struct AlwaysError {};

class Always : public FailureDetectorTag<AlwaysError>
{
public:
    NoState preRun()
    {
        return NoState();
    }

    template<typename T>
    failure postRun(NoState, ICallResult<T>& result)
    {
        if(result.isException())
        {
            result.consumeException();
        }
        return AlwaysError();
    }
};

}