#pragma once

namespace resilient {
namespace test {

// Make a reference const
template<typename T>
constexpr const T& as_const(T& t) noexcept
{
    return t;
}

} // namespace test
} // namespace resilient