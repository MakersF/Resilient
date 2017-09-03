#pragma once

#include <utility>
#include <tuple>

template<typename ...Args>
class AppendableTuple : public std::tuple<Args...>
{
private:
    using Base = std::tuple<Args...>;

    template<typename This, typename T>
    static std::tuple<Args..., T> append(This&& _this, T&& item)
    {
        return std::tuple_cat(std::forward<This>(_this),
                              std::make_tuple(std::forward<T>(item)));
    }

public:
    using Base::Base;

    template<typename T>
    std::tuple<Args..., T> append(T&& item) &
    {
        return append(*this, std::forward<T>(item));
    }

    template<typename T>
    std::tuple<Args..., T> append(T&& item) &&
    {
        return append(std::move(*this), std::forward<T>(item));
    }
}