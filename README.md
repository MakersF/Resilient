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

Find the [API documentation at ReadTheDocs](http://resilient.readthedocs.io/en/latest/).

# Requirements

- C++14
- boost (not required if using c++17)
- gcc-6 or newer (gcc-5 has a bug)

# Running the test

The project comes with some docker images to run the tests with different compilers and version of the standard.

Run `make test` in the project root to run all of the tests.

If you want to run the tests with a specific compiler and standard version you can do so.
For example run `make test-resilient-ubuntu16-gcc6-cpp17` to run on the image with ubuntu 16.04 LTS and gcc6 as default compiler, using `c++17` as standard.