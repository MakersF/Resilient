# Policies

A Policy defines how a function should be run.
They can be used to run a function multiple times (`Retry`) or to prevent them from running (`CircuitBreaker`) or to limit how often they are run (`RateLimiter`).
When you want to specify when and how to run a function a Policy can be used for that.

# The Protocol

A Policy is any callable which takes a callable as first arguments followed by a variadic list of arguments.
A typical signature is

```c++
template<typename Callable, typename ...Args>
decltype(auto) operator()(Callable&& callable, Args&&... args);
```

The `Callable` when invoked with `Args...` is expected to return a Failable.

There are no constraints on the return type of a Policy, but if a Policy returns a `Variant` it can be composed with other policies (see the `Pipeline` policy, which recursively invokes policies).


