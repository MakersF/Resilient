#include <chrono>
#include <iostream>

#include <resilient/detector/returns.hpp>
#include <resilient/policy/retry/factory/constructstate.hpp>
#include <resilient/policy/retry/retry.hpp>
#include <resilient/policy/retry/state/exponentialbackoff.hpp>
#include <resilient/task/task.hpp>

static auto last_time = std::chrono::steady_clock::now();

// Let's keep track of how long we waited between calls
std::chrono::microseconds from_last_time()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_time);
    last_time = now;
    return elapsed;
}

// Return true when the counter is zero.
// This is an example of an operation we want to retry.
bool countDown(int& counter)
{
    counter--;
    std::cout << "Counter: " << counter << ". Waited " << from_last_time().count() << " microsec"
              << std::endl;

    return counter == 0;
}

int main()
{
    using namespace std::chrono_literals;
    using resilient::returns;

    // Create a Retry class using a factory which calls the constructor of the retry state.
    // In this case we use an ExponentialBackoff as retry state with no limit on the number of
    // retries.
    auto statefactory =
        resilient::retry::constructstate<resilient::retry::ExponentialBackoff<>>(1ms, 5ms);
    auto retry = resilient::retry::retry(statefactory);

    // Our function is not a task as it does not return a Failable.
    // We can transform it to a task by adding the conditions in which it fails.
    auto task = resilient::task(&countDown).failsIf(returns(false));

    // We can execute the task as we could execute the function.
    // Execute takes any argument number of arguments, passing it to the task.
    int counter = 5;
    auto result = retry.execute(task, counter);

    // We then check if the result was successfull or it failed every time we retried it.
    if (resilient::holds_value(result)) {
        std::cout << "Result: " << resilient::get_value(result) << std::endl;
    }
    else
    {
        std::cout << "Exhausted the retries without getting a result." << std::endl;
    }
    return 0;
}