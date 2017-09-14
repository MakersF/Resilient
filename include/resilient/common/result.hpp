#ifndef RESILIENT_RESULT_H
#define RESILIENT_RESULT_H

#include <utility>
#include <boost/variant.hpp>

namespace resilient {

namespace detail {

template<typename T>
class IsType
    : public boost::static_visitor<bool>
{
public:

    bool operator()(const T&) const
    {
        return true;
    }

    template<typename Other>
    bool operator()(const Other&) const
    {
        return false;
    }
};

} // namespace detail

struct Failure {};

template<typename T>
bool isFailure(const T& result)
{
    return boost::apply_visitor(detail::IsType<Failure>(), result);
}

template<typename Result>
const Result& getResult(const boost::variant<Failure, Result>& result)
{
    return boost::strict_get<Result>(result);
}

template<typename Result>
struct ResultTraits
{
    using type = boost::variant<Failure, Result>;
    using result_type = Result;
    static constexpr bool is_result_type = false;
};

template<typename Result>
struct ResultTraits<boost::variant<Failure, Result>>
{
    using type = boost::variant<Failure, Result>;
    using result_type = Result;
    static constexpr bool is_result_type = true;
};

template<typename Callable, typename ...Args>
using ResultOf = typename ResultTraits<typename std::result_of<Callable(Args...)>::type>::type;

}

#endif