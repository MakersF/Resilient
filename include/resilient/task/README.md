# Task

Task is the class used by Resilient to associate a Detector to any callable.

Usually function do not return a Failable object, which indeed is required when calling using a Policy.

To bridge this the Task class can be used: given a callable and a Detector it will use the provided Detector to detect whether the call was succesful or not.

The result of Task is a Failable which can either contains the failure identified by the Detector or the value returned by the function.

If a function throws an exception a Detector is expected to consume it, otherwise it will be propagated.
This guarantees that exceptions are not silently swallowed.