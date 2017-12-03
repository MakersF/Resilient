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
     * @tparam T The type returned by the detected function.
     * @param ICallResult The result of invoking the detected function.
     * @return Always `NoFailure`.
     */
    template<typename T>
    returned_failure_t<failure_types> detect(ICallResult<T>&)
    {
        return NoFailure();
    }
};

}