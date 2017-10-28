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
    void postRun(NoState, const OperationResult<T>&, IFailureSignal<failure_types>& signal)
    {
        signal.signalFailure(AlwaysError());
    }
};

}