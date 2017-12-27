#pragma once

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

/**
 * @brief The type returned as failure by the `Always` detector.
 */
struct AlwaysError
{
};

/**
 * @ingroup Detector
 * @brief A detector which always detects failure.
 *
 */
class Always
: public FailureDetectorTag<AlwaysError>
, public StatelessDetector<Always>
{
public:
    /**
     * @brief Always detect AlwaysError failure and consume possible exceptions.
     *
     * @tparam T The type returned by the detected function.
     * @param result The result of invoking the detected function.
     * @return The failure detected: AlwaysError.
     */
    template<typename T>
    returned_failure_t<failure_types> detect(ICallResult<T>& result)
    {
        if (result.isException()) {
            // The goal of this class is to alwaws return error, independently from what happens.
            // We need to always consume the exception because if no one consumes it we are going to
            // get an error. Since there is no guarantee another detector will consume the exception
            // this needs to do it.
            result.consumeException();
        }
        return AlwaysError();
    }
};

} // namespace resilient