/**
 * @defgroup Detector
 *
 * @brief Detectors are the classes used by Resilient to determine whether a function failed or not.
 *
 * # Description
 *
 * There must be a way for the library for detecting when a function has failed.
 *
 * Common ways are to return a specific error code, throw an exception, and many other.
 * In order to allow you to define the failure conditions appropriate for your use case Resilient
 * uses `Detectors`.
 *
 * A detector is a class which is able to determine how a function behaved, and can return a
 * different type depending on the type of the failure.
 *
 *
 * # Glossary
 * A function which the return value will be used to detect a failure is called the `detected
 * function`.
 *
 * # The Concept
 *
 * Any class can be used as a detector as long as it implements the expected concept.
 *
 * A detector must define the types:
 * - `failure_types`: a tuple of the possible Failures this detector can detect.
 *
 * A detector must define the methods:
 * - `State preRun()`
 * - `Failure postRun(State&&, ICallResult<T>&)`
 *
 * Deriving from `FailureDetectorTag<MyFailure1, MyFailure2, ...>` defines the `failure_types` for
 * you.
 *
 * `preRun()` is called before the detected function is called.
 * The returned type `State` can be any type, and it will be moved back into the `postRun()` method
 * when invoked. The `State` type can be used to store some state the detector needs to keep between
 * calls to `preRun` and `postRun`.
 *
 * `postRun()` is called after the detected function is called.
 * The type returned by `preRun()` is moved into it as first argument and an instance of
 * `ICallResult` with the type returned by the detected function is passed as a second argument. `T`
 * is the type returned by the detected function. `Failure` needs to be a variant which containes
 * either `NoFailure` if no failure was detected or one of the types used in `failure_types` if a
 * failure was detected.
 *
 * `ICallResult` can be used to determine whether the function returned normally or threw an
 * exception and can also return a const reference to the value returned by the function or to the
 * threw exception.
 *
 * The detector can also "consume" an exception, which means that the exception should be considered
 * handled.
 *
 * # Resources
 *
 * The Detector is not required to be stateless, but being stateless heavily simplify writing a
 * detector as it prevents problems with multi-threading, it can be invoked multiple times and it
 * can also be used in different tasks.
 *
 * To make this simple the `preRun()` function can return an object of any type, which will be moved
 * back into the `postRun()` function.
 *
 * The detector can initialize any resource it needs inside the `preRun()` method and store it into
 * the state. The `postRun()` method can then make use of the resources initialized by `preRun()`
 * and release them at the end of the method.
 *
 * If the state object is the owner of the resources and RAII is used then the code has no clean-up
 * to do.
 *
 * If a Detector requires no state a `NoState` type can be used as return type.
 */

#include <resilient/detector/always.hpp>
#include <resilient/detector/any.hpp>
#include <resilient/detector/never.hpp>
#include <resilient/detector/returns.hpp>
#include <resilient/detector/throws.hpp>