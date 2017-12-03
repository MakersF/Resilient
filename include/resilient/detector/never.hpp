#pragma once

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

// Never detect failure
class Never : public FailureDetectorTag<>, public StatelessDetector<Never>
{
public:
    template<typename T>
    returned_failure_t<failure_types> detect(ICallResult<T>&)
    {
        return NoFailure();
    }
};

}