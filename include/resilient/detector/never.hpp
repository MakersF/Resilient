#pragma once

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

// Never detect failure
class Never : public FailureDetectorTag<>
{
public:
    NoState preRun()
    {
        return NoState();
    }

    template<typename T>
    returned_failure_t<failure_types> postRun(NoState, ICallResult<T>&)
    {
        return NoFailure();
    }
};

}