#pragma once

namespace resilient {

class Never
{
public:
    template<typename Callable, typename ...Args>
    auto operator()(Callable&& callable, Args&&... args)
    {
        return std::forward<Callable>(callable)(std::forward<Args>(args)...);
    }
};

}