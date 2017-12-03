#pragma once

#include <exception>
#include <utility>
#include <tuple>
#include <resilient/common/invoke.hpp>

namespace resilient {

/**
 * @brief Type representing that the detector does not need any state.
 */
struct NoState {};

/**
 * @ingroup Detector
 * @brief Interface to represent the result of calling a detected function.
 *
 * A detected function invokation can result in either:
 * - returning a result
 * - throwing an exception
 *
 * @tparam T The type returned by the detected function.
 */
template<typename T>
class ICallResult
{
public:
    using ConstRefType = const std::decay_t<T>&;

    /**
     * @brief Consume (signal as handled) an exception if the detected threw one.
     *
     * It's invalid to call this function if `isException()` returns false.
     */
    virtual void consumeException() = 0;

    /**
     * @brief Whether the detected function invokation threw an exception.
     *
     * @return true An exception was thrown.
     * @return false An result was returned
     */
    virtual bool isException() const = 0;

    /**
     * @brief Get a pointer of the thrown exception.
     *
     * It's invalid to call this function is `isException()` returns false.
     *
     * @return const std::exception_ptr&
     */
    virtual const std::exception_ptr& getException() const = 0;

    /**
     * @brief Access the result of the detected function.
     *
     * It's invalid to call this function if `isException()` returns true.
     *
     * @return ConstRefType The value returned by the detected function.
     */
    virtual ConstRefType getResult() const = 0;

    virtual ~ICallResult() {}
};

/**
 * @ingroup Detector
 * @brief Visit the `ICallResult<T>` as if it was a variant.
 *
 * @tparam Visitor The type of the object which defines the overloaded `operator()` to be called.
 * @tparam T The type returned by the detected function.
 * @param visitor The object called with either the `std::exception_ptr` or `const T&`.
 * @param callresult The callresult object.
 * @return The type returned by the visitor.
 */
template<typename Visitor, typename T>
constexpr decltype(auto) visit(Visitor&& visitor, ICallResult<T>& callresult)
{
    if (callresult.isException())
    {
        return detail::invoke(std::forward<Visitor>(visitor), callresult.getException());
    }
    else
    {
        return detail::invoke(std::forward<Visitor>(visitor), callresult.getResult());
    }
}

}