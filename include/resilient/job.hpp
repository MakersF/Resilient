#include <utility>
#include <tuple>
#include <type_traits>
#include <boost/variant.hpp>
#include <exception>
#include <resilient/result.hpp>

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

template<typename Callable, typename ...Args>
class LValCall
{
public:
    using result_type = ResultOf<Callable&, Args&...>;

    LValCall(Callable& callable, Args&... args)
    : d_callable(callable)
    , d_args(args...)
    { }

    result_type run() { return apply(d_callable, d_args); }

private:
    Callable& d_callable;
    std::tuple<Args&...> d_args;
};

struct Job
{

};




//////////////////////////////////////////////////////////


struct RetryPolicy
{
    int times;

    template<typename Job>
    JobResult<Job> run(Job&& job)
    {
        JobResult<Job> result = Failure{};
        for(int i = 0; i < times; i++)
        {
            result = job.run();
            if(not isFailure(result))
            {
                return std::move(result);
            }
        }
        return result;
    }
};

struct CircuitBreak
{
    bool open;

    template<typename Job>
    JobResult<Job> run(Job&& job)
    {
        if(not open)
        {
            return job.run();
        }
        return JobResult<Job>{false};
    }
};

}