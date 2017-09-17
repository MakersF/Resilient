#pragma once

#include <utility>
#include <boost/variant.hpp>

namespace resilient {

// TODO
// T -> F(T)
// F(T) -> T
// g(x) -> g(F(x))
// g(F(x)) -> g(x)

namespace detail {

/*
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
*/
} // namespace detail

template<typename Failure, typename T>
struct Failable : public boost::variant<Failure, T>
{
private:
    using Base = boost::variant<Failure, T>;

public:
    using Base::Base; // Constructor inheritance
};

template<typename Failure, typename T>
using failable_t = Failable<Failure, T>;

template<typename Failure, typename T>
failable_t<Failure, T> make_failable(T&& value)
{
    return failable_t<Failure, T>{std::forward<T>(value)};
}

template<typename Failable>
struct FailableTraits
{
    static constexpr bool is_failable = false;
};

// Value specialization (both constness)
template<typename Failure, typename T>
struct FailableTraits<Failable<Failure, T>>
{
    using FailableType = failable_t<Failure, T>;

    static constexpr bool is_failable = true;

    static inline bool isFailure(const FailableType& failable)
    {
        return boost::apply_visitor(detail::IsType<Failure>(), failable);
    }

    static inline bool isSuccess(const FailableType& failable)
    {
        return !isFailure(failable);
    }

    static inline const T& getValue(const FailableType& failable)
    {
        return boost::strict_get<T>(failable);
    }

    static inline T getValue(FailableType&& failable)
    {
        return boost::strict_get<T>(std::move(failable));
    }

    static FailableType failure()
    {
        return FailableType();
    }
};

template<typename Failure, typename T>
struct FailableTraits<Failable<Failure, T> const > : FailableTraits<Failable<Failure, T>> { };

// Lvalue references specialization (both constness)
template<typename Failure, typename T>
struct FailableTraits<Failable<Failure, T> &> : FailableTraits<Failable<Failure, T>> { };

template<typename Failure, typename T>
struct FailableTraits<Failable<Failure, T> const &> : FailableTraits<Failable<Failure, T>> { };

// Rvalue references specialization (both constness)
template<typename Failure, typename T>
struct FailableTraits<Failable<Failure, T> &&> : FailableTraits<Failable<Failure, T>> { };

template<typename Failure, typename T>
struct FailableTraits<Failable<Failure, T> const &&> : FailableTraits<Failable<Failure, T>> { };

}