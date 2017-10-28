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
    void postRun(NoState, const OperationResult<T>&, IFailureSignal<failure_types>&) { }
};

}