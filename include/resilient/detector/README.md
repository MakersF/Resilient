# Detectors

Detectors are the classes used by Resilient to determine whether a function failed or not.
A function which the return value will be used to detect a failure is called the `detected function`.

# The protocol

A detector is expected to implement 2 methods:
- `State preRun()`
- `Failure postRun(State&&, ICallResult<T>&)`

`preRun()` is called before the detected function is called.
The returned type can be any type, and it will be moved back into the `postRun()` method when invoked.

`postRun()` is called after the detected function is called.
The type returned by `preRun()` is moved into it as first argument and an instance of `ICallResult` with the type returned byt the function is passed as a second argument.

`ICallResult` can be used to determine whether the function returned normally or threw an exception and can also return a const reference to the value returned by the function or to the threw exception.

The detector can also "consume" an exception, which means that the exception should be considered handled.

The detector is expected to return a `Failure` type: it must be a `Variant` which has as possible states `NoFailure` if no failure is detected or an instance of the failure the detector is able to detect.


# Resources

The Detector is not required to be stateless, but being stateless heavily simplify writing a detector as it prevents problems with multi-threading, it can be invoked multiple times and it can also be used in different tasks.

To make this simple the `preRun()` function can return an object of any type, which will be moved back into the `postRun()` function.

The detector can initialize any resource it needs inside the `preRun()` method and store it into the state.
The `postRun()` method can then make use of the resources initialized by `preRun()` and release them at the end of the method.

If the state object is the owner of the resources and RAII is used then the code has no clean-up to do.

If a Detector requires no state a `NoState` type can be used as return type.