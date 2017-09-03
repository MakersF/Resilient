#include <utility>
#include <tuple>
#include <type_traits>
#include <boost/variant.hpp>
#include <exception>
#include <resilient/result.hpp>
#include <resilient/appendable_tuple.hpp>
#include <resilient/foldinvoke.hpp>
#include <iostream>

namespace resilient {

/////////// TODO refactor
template<size_t N>
struct Apply {
    template<typename F, typename T, typename... A>
    static inline auto apply(F && f, T && t, A &&... a) {
        return Apply<N-1>::apply(::std::forward<F>(f), ::std::forward<T>(t),
            ::std::get<N-1>(::std::forward<T>(t)), ::std::forward<A>(a)...
        );
    }
};

template<>
struct Apply<0> {
    template<typename F, typename T, typename... A>
    static inline auto apply(F && f, T &&, A &&... a) {
        return ::std::forward<F>(f)(::std::forward<A>(a)...);
    }
};

template<typename F, typename T>
inline auto apply(F && f, T && t) {
    return Apply< ::std::tuple_size< ::std::decay_t<T>
      >::value>::apply(::std::forward<F>(f), ::std::forward<T>(t));
}
///////////

template<typename Job>
using JobResult = typename std::decay<Job>::type::result_type;

template<typename Policy>
struct PolicyTraits
{
private:
    using FunReturningResult = ResultTraits<int>::type (*) ();
    using PolicyResult = std::result_of_t<Policy(FunReturningResult)>;

public:
    static constexpr bool is_valid_policy =
        ResultTraits<PolicyResult>::is_result_type;
};

template<typename Callable, typename ...Args>
class LValCall
{
public:
    using result_type = ResultOf<Callable&, Args&...>;

    LValCall(Callable& callable, Args&... args)
    : d_callable(callable)
    , d_args(args...)
    { }

    result_type operator()()
    {
        // return apply(d_callable, d_args);
        return Failure{};
    }

private:
    Callable& d_callable;
    std::tuple<Args&...> d_args;
};

template<typename ...Args>
struct AsFailable
{
public:

    AsFailable(Args&... args)
    : d_args(args...)
    { }

    template<typename Callable>
    decltype(auto) operator()(Callable&& callable)
    {
        return apply(std::forward<Callable>(callable), d_args);
    }

private:
    std::tuple<Args&...> d_args;
};

template<typename ...Policies>
class Job
{
public:
    template<typename Policy>
    Job<Policies... , Policy> then(Policy&& policy)
    {
        static_assert(PolicyTraits<Policy>::is_valid_policy, "Not a valid policy");

        return Job<Policies... , Policy>(
            tuple_append(d_policies, std::forward<Policy>(policy)));
    }

    template<typename Policy>
    static Job<Policy> with(Policy&& policy)
    {
        return Job<Policy>(std::tuple<Policy>(std::forward<Policy>(policy)));
    }

    template<typename Call, typename ...Args>
    decltype(auto) run(Call&& call, Args&&... args)
    {
        foldInvoke(d_policies, LValCall<Call, Args...>(call, args...));
    }

private:

    std::tuple<Policies...> d_policies;

    explicit Job(std::tuple<Policies...>&& policies)
    : d_policies(std::move(policies))
    { }

    template<typename ...T>
    friend class Job;
};


//////////////////////////////////////////////////////////

struct RetryPolicy
{
    int times;

    template<typename Job>
    auto operator()(Job&& job) -> std::result_of_t<Job()>
    {
        for(int i = 0; i < times; i++)
        {
            std::cout << "Retry " << i << " " << std::flush;
            auto&& result = job();
            if(not isFailure(result))
            {
                return std::move(result);
            }
        }
        return Failure{};
    }
};

struct Monitor
{
    template<typename Job>
    auto operator()(Job&& job) -> std::result_of_t<Job()>
    {
        std::cout << "Before " << std::flush;
        auto ret = job();
        std::cout << "After " << std::flush;
        return std::move(ret);
    }
};

struct CircuitBreak
{
    bool open = false;

    template<typename Job>
    auto operator()(Job&& job) -> std::result_of_t<Job()>
    {
        if(open)
        {
            std::cout << "Open " << std::flush;
            return Failure{};
        }
        std::cout << "Closed " << std::flush;
        return job();
    }
};

}