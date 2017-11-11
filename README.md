# Resilient

Resilient is a library to simplify habdling failures which might happen when performing opertations.

In a world of connected services it's not easy to take into account that this services are not always available.

Resilient aims to ease the work required to do that.

 ```c++
with(RetryPolicy(5))
    .run(task([start = 0] () mutable { std::cout << "Run: " << start << std::endl; return start++; })
    .failsIf(Any(Returns<int>(0), Returns<int>(1)));
 ```

 Will print
 ```
 Run: 0
 Run: 1
 Run: 2
 ```


# Documentation

More documentation can be found in the commomn, detector, policy and task folders inside the resilient folder.
# Requirements

- C++14
- gcc-6 or newer
- boost