#pragma once

#include <exception>

#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>

namespace resilient {

/**
 * @brief Type returned by Throws when the detected function throws the expected exception.
 */
template<typename T>
struct ExceptionThrown
{
    /**
     * @brief The pointer to the exception thrown.
     */
    std::exception_ptr value;
};

/**
 * @ingroup Detector
 * @brief A detector which detect whether a function throws an exception of a given type.
 *
 * If the detected function throws an exception of the expected type then this class
 * returns `ExceptionThrown`.
 *
 * @note
 * This detector also matches exceptions derived from the expected type.
 * If an exception type `B` derives from `A`, `Throws<A>` returns a failure if an instance of
 * `B` is thrown.
 *
 * @warning
 * When using several `Throws` in an `Any` detector be sure to put instance especting the
 * most specific exception first and the least specific exception last, otherwise the latter
 * will always detect the exception and the most specific one will never be called.
 *
 *
 * @tparam T The type of the exception.
 *
 */
template<typename T>
class Throws
: public FailureDetectorTag<ExceptionThrown<T>>
, public StatelessDetector<Throws<T>>
{
public:
    /**
     * @brief Check whether the result contains an exception of the expected type.
     *
     * @tparam Q The return type of the detected function
     * @param result The result of invoking the detected function.
     * @return `ExceptionThrown` if an exception occurred and the type is the expected one,
     * `NoFailure` otherwise.
     */
    template<typename Q>
    returned_failure_t<typename FailureDetectorTag<ExceptionThrown<T>>::failure_types>
        detect(ICallResult<Q>& result)
    {
        if (result.isException()) {
            std::exception_ptr exception = result.getException();
            try
            {
                std::rethrow_exception(exception);
            }
            catch (const T&)
            {
                return ExceptionThrown<T>{exception};
            }
            catch (...)
            {
                return NoFailure();
            }
        }
        else
        {
            return NoFailure();
        }
    }
};

} // namespace resilient