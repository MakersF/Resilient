#pragma once

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

/**
 * @brief Type returned by Returns when the detected function returns the expected value.
 */
struct ErrorReturn
{
};

/**
 * @ingroup Detector
 * @brief A detector which detect whether a function returns a specific value.
 *
 * If the detected function returns the specified value then this class returns `ErrorReturn`.
 *
 * @tparam T The type of the value used to compare against the returned value of the function.
 *
 * @note The type `T` can be different from the returned type, as long as they can be compared for
 * equality.
 */
template<typename T>
class Returns
: public FailureDetectorTag<ErrorReturn>
, public StatelessDetector<Returns<T>>
{
public:
    /**
     * @brief Construct an instance of Returns.
     *
     * @param failureValue The value used to compare against the return value of the function.
     */
    Returns(T&& failureValue) : d_failureValue(std::forward<T>(failureValue)) {}

    /**
     * @brief Check whether the result contains the expected value.
     *
     * @tparam Q The return type of the detected function
     * @param result The result of invoking the detected function.
     * @return `ErrorReturn` if the returned type is equal to the expected type, `NoFailure`
     * otherwise.
     */
    template<typename Q>
    returned_failure_t<failure_types> detect(ICallResult<Q>& result)
    {
        if (not result.isException() and d_failureValue == result.getResult()) {
            return ErrorReturn();
        }
        else
        {
            return NoFailure();
        }
    }

private:
    T d_failureValue;
};

/**
 * @brief Return an instance of `Returns` with the provided value
 * @related resilient::Returns
 */
template<typename T>
Returns<T> returns(T&& value)
{
    return Returns<T>(std::forward<T>(value));
}

} // namespace resilient