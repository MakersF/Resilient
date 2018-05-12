#pragma once

#include <chrono>
#include <memory>
#include <mutex>

#include <resilient/common/variant.hpp>
#include <resilient/detail/variant_utils.hpp>
#include <resilient/policy/circuitbreaker.hpp>

namespace resilient {

/**
 * @brief Open the circuit breaker if a number of failures happen over a time intervall.
 * @related resilient::ICircuitbreakerStrategy
 *
 * @tparam Clock The kind of clock to use when measuring time.
 */
template<typename Clock = std::chrono::steady_clock>
class CountStrategy : ICircuitbreakerStrategy
{
public:
    /**
     * @brief Construct a new Count Strategy object.
     *
     *
     * @param failures Number of failures over the interval which will trigger the circuit breaker.
     * @param failureInterval The duration of the interval over which to count the failures.
     * @param tripDuration How long to keep the circuit breaker open once it trips.
     * @param recoverSuccesses How many task executions should succeed before resetting the
     *                         circuit breaker to it's normal state after it triggered.
     * @param clock An instance of the clock to use to measure time. Defaults to a default initialized one.
     */
    CountStrategy(unsigned long failures,
                  std::chrono::microseconds failureInterval,
                  std::chrono::microseconds tripDuration,
                  unsigned long recoverSuccesses,
                  Clock clock = Clock());

    bool allowCall() override
    {
        std::lock_guard<std::mutex> guard{d_stateMutex};
        return reentrantAllowCall();
    }

    void registerFailure() override
    {
        std::lock_guard<std::mutex> guard{d_stateMutex};
        reentrantRegisterFailure();
    }

    void registerSuccess() override
    {
        std::lock_guard<std::mutex> guard{d_stateMutex};
        reentrantRegisterSuccess();
    }

private:
    using time_point = typename Clock::time_point;

    // Reentrant version of the public methods.
    // These can be called by the states hold in the variant
    bool reentrantAllowCall();
    void reentrantRegisterFailure();
    void reentrantRegisterSuccess();

    // Equivalent to Closed state.
    // State changes:
    //  - when enough task executions fail -> switch to Intercept
    struct LetThrough
    {
        LetThrough(CountStrategy* parent, time_point firstFailure)
        : d_parent(parent), d_intervalStart(firstFailure), d_failureCounter(0)
        {
        }

        bool allowCall() { return true; }
        void registerSuccess() {}
        void registerFailure();

    private:
        void resetIfIntervalExpired(time_point now);

        CountStrategy* d_parent;
        time_point d_intervalStart;
        unsigned long d_failureCounter;
    };

    // Equivalent to Open state.
    // State changes:
    // - when tripDuration has elapsed -> switch to TryLetThrough
    struct Intercept
    {

        Intercept(CountStrategy* parent, time_point now)
        : d_parent(parent), d_endTime(now + d_parent->d_tripDuration)
        {
        }

        bool allowCall();

        // Out expectation would be that these functions are never called.
        // But they might be called in a multi-threaded environment, as 1
        // thread might switch the strategy to Intercept while the other
        // passed already the check while it was in LetThrough.
        void registerFailure() {}
        void registerSuccess() {}

    private:
        CountStrategy* d_parent;
        time_point d_endTime;
    };

    //Equivalent to Half-Open state.
    // State changes:
    // - when the next task execution succeeds -> switch to LetThrough
    // - when the next task execution fails -> switch to Intercept
    struct TryLetThrough
    {

        TryLetThrough(CountStrategy* parent) : d_parent(parent), d_successfulRequests(0) {}

        bool allowCall() { return true; }
        void registerFailure();
        void registerSuccess();

    private:
        CountStrategy* d_parent;
        unsigned long d_successfulRequests;
    };

    // Switch the current state
    // NOTE: After calling this function from one of the structs inside the
    //       variant NO MEMBER STATE CAN BE ACCESSED. The struct has been
    //       destructed!
    //       If you need to access state, copy it into the local variables.
    template<typename T>
    void switchTo(T obj)
    {
        d_state = std::move(obj);
    }

    unsigned long d_failuresPerInterval;
    std::chrono::microseconds d_failureInterval;
    std::chrono::microseconds d_tripDuration;
    unsigned long d_successesBeforeRecovering;
    Clock d_clock;

    // Lock around changes to the variant
    std::mutex d_stateMutex;
    Variant<LetThrough, Intercept, TryLetThrough> d_state;
};

template<typename Clock>
CountStrategy<Clock>::CountStrategy(unsigned long failures,
                                    std::chrono::microseconds failureInterval,
                                    std::chrono::microseconds tripDuration,
                                    unsigned long recoverSuccesses,
                                    Clock clock)
: d_failuresPerInterval(failures)
, d_failureInterval(failureInterval)
, d_tripDuration(tripDuration)
, d_successesBeforeRecovering(recoverSuccesses)
, d_clock(std::move(clock))
, d_state(LetThrough(this, d_clock.now()))
{
}

template<typename Clock>
bool CountStrategy<Clock>::reentrantAllowCall()
{
    return visit(detail::overload<bool>([](auto& obj) { return obj.allowCall(); }), d_state);
}

template<typename Clock>
void CountStrategy<Clock>::reentrantRegisterFailure()
{
    visit(detail::overload<void>([](auto& obj) { obj.registerFailure(); }), d_state);
}

template<typename Clock>
void CountStrategy<Clock>::reentrantRegisterSuccess()
{
    visit(detail::overload<void>([](auto& obj) { obj.registerSuccess(); }), d_state);
}

template<typename Clock>
void CountStrategy<Clock>::LetThrough::resetIfIntervalExpired(time_point now)
{
    auto intervalEnd = d_intervalStart + d_parent->d_failureInterval;
    if (now > intervalEnd) {
        d_intervalStart = now;
        d_failureCounter = 0;
    }
}

template<typename Clock>
void CountStrategy<Clock>::LetThrough::registerFailure()
{
    auto now = d_parent->d_clock.now();

    resetIfIntervalExpired(now);

    d_failureCounter++;

    if (d_failureCounter >= d_parent->d_failuresPerInterval) {
        d_parent->switchTo(Intercept(d_parent, now));
    }
}

template<typename Clock>
bool CountStrategy<Clock>::Intercept::allowCall()
{
    if (d_parent->d_clock.now() >= d_endTime) {
        CountStrategy<Clock>* parent = d_parent;
        d_parent->switchTo(TryLetThrough(d_parent));
        return parent->reentrantAllowCall();
    }
    return false;
}

template<typename Clock>
void CountStrategy<Clock>::TryLetThrough::registerFailure()
{
    d_parent->switchTo(Intercept(d_parent, d_parent->d_clock.now()));
}

template<typename Clock>
void CountStrategy<Clock>::TryLetThrough::registerSuccess()
{
    d_successfulRequests++;
    if (d_successfulRequests >= d_parent->d_successesBeforeRecovering) {
        d_parent->switchTo(LetThrough(d_parent, d_parent->d_clock.now()));
    }
}

} // namespace resilient