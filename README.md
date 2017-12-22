# Resilient

Resilient is a library to simplify handling failures which might happen when performing operations.

In a world of connected services it's not easy to assume that this services are not always available.

Resilient aims to ease the work required to do that.

 ```c++
auto counter = [start = 0] () mutable { std::cout << "Run: " << start << std::endl; return start++; };
RetryPolicy(5).execute(task(counter).failsIf(Any(Returns<int>(0), Returns<int>(1)));
 ```

 Will print
 ```
 Run: 0
 Run: 1
 Run: 2
 ```


# Documentation

Most of the code has documentation in line.

Work is in progress to generated HTML documentation with doxygen.

# Requirements

- C++14
- gcc-6 or newer (gcc-5 has a bug)
- boost (not required if using c++17)
