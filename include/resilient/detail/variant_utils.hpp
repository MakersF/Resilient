#pragma once

#include <cassert>
#include <resilient/common/variant.hpp>
#include <type_traits>

namespace resilient {
namespace detail {

template<typename T, typename Q>
using is_decayed_same = std::is_same<std::decay_t<T>, Q>;

template<typename T, typename Q>
using if_is_decayed_same = std::enable_if_t<is_decayed_same<T, Q>::value, void*>;

template<typename T, typename Q>
using if_is_not_decayed_same = std::enable_if_t<not is_decayed_same<T, Q>::value, void*>;

// Create a visitor which forwards the call to the visitor it wraps except for a specific type.
// Usefull when you want to perform operations on a variant while knowing one of the possible types
// it contains is not the current value.
template<typename IgnoreType, typename Wrapped>
struct IgnoreTypeVisitor
{
    using result_type = typename std::decay_t<Wrapped>::result_type;
    Wrapped d_wrapped;

    template<typename T, if_is_decayed_same<T, IgnoreType> = nullptr>
    result_type operator()(T&&)
    {
        assert(false); // The ignored type should never be contained by the variant
    }

    template<typename T, if_is_not_decayed_same<T, IgnoreType> = nullptr>
    result_type operator()(T&& value)
    {
        return detail::invoke(std::forward<Wrapped>(d_wrapped), std::forward<T>(value));
    }
};

template<typename IgnoreType, typename Wrapped>
IgnoreTypeVisitor<IgnoreType, Wrapped> make_ignoretype(Wrapped&& wrapped)
{
    return IgnoreTypeVisitor<IgnoreType, Wrapped>{std::forward<Wrapped>(wrapped)};
}

// Assign the current value of a variant to another variant
template<typename Destination>
struct AssignVisitor
{
    // this is required because boost (possible implementation of variant) requires it from
    // visitors.
    using result_type = void;
    Destination& d_destinationVariant;

    template<typename T>
    void operator()(T&& value)
    {
        d_destinationVariant = std::forward<T>(value);
    }
};

template<typename Destination, typename Source>
void assign_from_variant(Destination& destination, Source&& source)
{
    visit(AssignVisitor<Destination>{destination}, std::forward<Source>(source));
}

// Construct an object with the current value of a variant
template<typename ConstructedType>
struct ConstructVisitor
{
    // this is required because boost (possible implementation of variant) requires it from
    // visitors.
    using result_type = ConstructedType;

    template<typename T>
    ConstructedType operator()(T&& value)
    {
        return ConstructedType{std::forward<T>(value)};
    }
};

template<typename Type, typename Source>
Type construct_from_variant(Source&& source)
{
    return visit(ConstructVisitor<Type>{}, std::forward<Source>(source));
}

template<typename F1, typename F2>
struct Overloaded
: F1
, F2
{
    Overloaded(F1&& f1, F2&& f2) : F1(std::forward<F1>(f1)), F2(std::forward<F2>(f2)) {}

    using F1::operator();
    using F2::operator();
};

// Add the result_type using to a visitor
template<typename ResultType, typename F>
struct AddResultTypeAlias : F
{
    using result_type = ResultType;

    AddResultTypeAlias(F&& f) : F(std::forward<F>(f)) {}

    template<typename... Args>
    result_type operator()(Args&&... args)
    {
        return (*static_cast<F*>(this))(std::forward<Args>(args)...);
    }
};

// Create a visitor from two callable (lambdas, ...)
template<typename ResultType, typename F1, typename F2>
AddResultTypeAlias<ResultType, Overloaded<F1, F2>> overload(F1&& f1, F2&& f2)
{
    return {Overloaded<F1, F2>{std::forward<F1>(f1), std::forward<F2>(f2)}};
}

} // namespace detail
} // namespace resilient