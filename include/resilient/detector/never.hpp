#pragma once

#include <resilient/detector/basedetector.hpp>

namespace resilient {

class Never : public FailureDetectorTag<>
{
public:
    template<typename Callable, typename ...Args>
    auto operator()(Callable&& callable, Args&&... args)
    {
        return std::forward<Callable>(callable)(std::forward<Args>(args)...);
    }
};

}