#pragma once

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

/**
 * @ingroup Detector
 * @brief A detector which never detects failure.
 */
class Never : public FailureDetectorTag<>, public StatelessDetector<Never>
{
public:
    /**
     * @brief %Never detect a failure.
     *
     * @return Always `NoFailure`.
     */
    template<typename T>
    returned_failure_t<failure_types> detect(ICallResult<T>&)
    {
        return NoFailure();
    }
};

}