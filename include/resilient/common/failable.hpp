#pragma once

#include <utility>
#include <cassert>

#include <boost/variant.hpp>

#include <resilient/common/utilities.hpp>

namespace resilient {

// TODO
// T -> F(T)
// F(T) -> T
// g(x) -> g(F(x))
// g(F(x)) -> g(x)

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

template<typename Failure, typename T>
class Failable
{
    // if we derive publicly from boost::variant some weird overload resolution failures error come up
    // when using gtest
private:
    template<typename Class>
    static constexpr bool is_failable = std::is_same<std::decay_t<Class>, Failable>::value;

public:
    template<typename = std::enable_if_t<std::is_default_constructible<Failure>::value>>
    Failable() : d_data() { }

    // Constructors
    template<typename Other,
             typename std::enable_if_t<is_failable<Other>, void*> = nullptr>
    Failable(Other&& failable)
    : d_data(forward_as<Other>(failable.d_data))
    { }

    template<typename Other,
             typename std::enable_if_t<!is_failable<Other>, void*> = nullptr>
    Failable(Other&& value)
    : d_data(std::forward<Other>(value))
    { }

    // Assignments
    template<typename Other,
             typename std::enable_if_t<is_failable<Other>, void*> = nullptr>
    Failable& operator=(Other&& failable)
    {
        d_data = forward_as<Other>(failable.d_data);
        return *this;
    }

    template<typename Other,
             typename std::enable_if_t<!is_failable<Other>, void*> = nullptr>
    Failable& operator=(Failure&& failure)
    {
        d_data = std::forward<Other>(failure);
        return *this;
    }

private:
    boost::variant<Failure, T> d_data;

    boost::variant<Failure, T>& operator*() & { return d_data; }
    const boost::variant<Failure, T>& operator*() const & { return d_data; }
    boost::variant<Failure, T>&& operator*() && { return std::move(d_data); }

    template<typename>
    friend struct FailableTraits;
};

template<typename Failure, typename T>
Failable<Failure, T> make_failable(T&& value)
{
    return Failable<Failure, T>(std::forward<T>(value));
}

template<typename T, typename Failure>
Failable<Failure, T> failure(Failure&& failure)
{
    return Failable<Failure, T>(std::forward<Failure>(failure));
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
    // TODO move everything as a free function
    using FailableType = Failable<Failure, T>;

    static constexpr bool is_failable = true;

    static inline bool isFailure(const FailableType& failable)
    {
        return boost::apply_visitor(detail::IsType<Failure>(), *failable);
    }

    static inline bool isSuccess(const FailableType& failable)
    {
        return !isFailure(failable);
    }

    static inline const T& getValue(const FailableType& failable)
    {
        assert(isSuccess(failable));
        return boost::strict_get<T>(*failable);
    }

    static inline T getValue(FailableType&& failable)
    {
        assert(isSuccess(failable));
        return boost::strict_get<T>(*std::move(failable));
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

template<typename Failable>
auto traits(const Failable&)
{
    return FailableTraits<std::decay_t<Failable>>();
}

}