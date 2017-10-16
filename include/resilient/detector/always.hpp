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
    void postRun(NoState, OperationResult<T>, FailureSignal<failure_types>& signal)
    {
        signal.signalFailure(AlwaysError());
    }
};

}