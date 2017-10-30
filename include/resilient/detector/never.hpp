#pragma once

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/execution_context.hpp>

namespace resilient {

class Never : public FailureDetectorTag<>
{
public:
    NoState preRun()
    {
        return NoState();
    }

    template<typename T>
    failure postRun(NoState, ICallResult<T>&)
    {
        return NoFailure();
    }
};

}