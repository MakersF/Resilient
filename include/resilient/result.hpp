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

template<typename T>
struct ResultTraits
{
    using type = boost::variant<Failure, T>;
    static constexpr bool is_result_type = false;
};

template<typename ...Args>
struct ResultTraits<boost::variant<Failure, Args...>>
{
    using type = boost::variant<Failure, Args...>;
    static constexpr bool is_result_type = true;
};

template<typename Callable, typename ...Args>
using ResultOf = typename ResultTraits<typename std::result_of<Callable(Args...)>::type>::type;

}

#endif