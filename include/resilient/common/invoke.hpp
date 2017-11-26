#pragma once

#include <type_traits>

#if __cplusplus >= 201703L
#include <functional>
#endif

namespace resilient {
namespace detail {

#if __cplusplus >= 201703L

using std::invoke_result;
using std::invoke_result_t;
using std::is_invocable;
using std::is_invocable_r;
using std::is_nothrow_invocable;
using std::is_nothrow_invocable_r;
using std::invoke;

#else

namespace impl {

template <typename T>
struct is_reference_wrapper : std::false_type {};
template <typename U>
struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};

template <typename Base, typename T, typename Derived, typename... Args>
auto INVOKE(T Base::*pmf, Derived&& ref, Args&&... args)
noexcept(noexcept((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...)))
 -> typename std::enable_if<std::is_function<T>::value &&
        std::is_base_of<Base, typename std::decay<Derived>::type>::value,
    decltype((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...))>::type {
    return (std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...);
}
template <typename Base, typename T, typename RefWrap, typename... Args>
auto INVOKE(T Base::*pmf, RefWrap&& ref, Args&&... args)
noexcept(noexcept((ref.get().*pmf)(std::forward<Args>(args)...)))
 -> typename std::enable_if<std::is_function<T>::value &&
        is_reference_wrapper<typename std::decay<RefWrap>::type>::value,
    decltype((ref.get().*pmf)(std::forward<Args>(args)...))>::type {
    return (ref.get().*pmf)(std::forward<Args>(args)...);
}
template <typename Base, typename T, typename Pointer, typename... Args>
auto INVOKE(T Base::*pmf, Pointer&& ptr, Args&&... args)
noexcept(noexcept(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...)))
 -> typename std::enable_if<std::is_function<T>::value &&
        !is_reference_wrapper<typename std::decay<Pointer>::type>::value &&
        !std::is_base_of<Base, typename std::decay<Pointer>::type>::value,
    decltype(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...))>::type {
    return ((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...);
}
template <typename Base, typename T, typename Derived>
auto INVOKE(T Base::*pmd, Derived&& ref)
noexcept(noexcept(std::forward<Derived>(ref).*pmd))
 -> typename std::enable_if<!std::is_function<T>::value &&
        std::is_base_of<Base, typename std::decay<Derived>::type>::value,
    decltype(std::forward<Derived>(ref).*pmd)>::type {
    return std::forward<Derived>(ref).*pmd;
}
template <typename Base, typename T, typename RefWrap>
auto INVOKE(T Base::*pmd, RefWrap&& ref)
noexcept(noexcept(ref.get().*pmd))
 -> typename std::enable_if<!std::is_function<T>::value &&
        is_reference_wrapper<typename std::decay<RefWrap>::type>::value,
    decltype(ref.get().*pmd)>::type {
    return ref.get().*pmd;
}
template <typename Base, typename T, typename Pointer>
auto INVOKE(T Base::*pmd, Pointer&& ptr)
noexcept(noexcept((*std::forward<Pointer>(ptr)).*pmd))
 -> typename std::enable_if<!std::is_function<T>::value &&
        !is_reference_wrapper<typename std::decay<Pointer>::type>::value &&
        !std::is_base_of<Base, typename std::decay<Pointer>::type>::value,
    decltype((*std::forward<Pointer>(ptr)).*pmd)>::type {
    return (*std::forward<Pointer>(ptr)).*pmd;
}
template <typename F, typename... Args>
auto INVOKE(F&& f, Args&&... args)
noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
 -> typename std::enable_if<!std::is_member_pointer<typename std::decay<F>::type>::value,
    decltype(std::forward<F>(f)(std::forward<Args>(args)...))>::type {
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

template <typename AlwaysVoid, typename, typename...>
struct invoke_result {};
template <typename F, typename... Args>
struct invoke_result<decltype(void(INVOKE(std::declval<F>(),
        std::declval<Args>()...))), F, Args...> {
    using type = decltype(INVOKE(std::declval<F>(), std::declval<Args>()...));
};

template <typename AlwaysVoid, typename, typename...>
struct is_invocable : std::false_type {};
template <typename F, typename... Args>
struct is_invocable<decltype(void(INVOKE(std::declval<F>(),
        std::declval<Args>()...))), F, Args...> : std::true_type {};

template <bool cond, typename F, typename... Args>
struct is_nothrow_invocable : std::false_type {};
template <typename F, typename... Args>
struct is_nothrow_invocable<true, F, Args...> : std::integral_constant<bool,
        noexcept(INVOKE(std::declval<F>(), std::declval<Args>()...))> {};

}  // namespace impl

template <typename F, typename... Args>
struct invoke_result : impl::invoke_result<void, F, Args...> {};
template<typename F, typename... Args>
using invoke_result_t = typename invoke_result<F, Args...>::type;

template <typename F, typename... Args>
struct is_invocable : impl::is_invocable<void, F, Args...> {};
template <typename R, typename F, typename... Args>
struct is_invocable_r : std::integral_constant<bool,
    is_invocable<F, Args...>::value &&
        std::is_convertible<invoke_result_t<F, Args...>, R>::value> {};
template <typename F, typename... Args>
struct is_nothrow_invocable :
    impl::is_nothrow_invocable<is_invocable<F, Args...>::value, F, Args...> {};
template <typename R, typename F, typename... Args>
struct is_nothrow_invocable_r : std::integral_constant<bool,
    is_nothrow_invocable<F, Args...>::value &&
        std::is_convertible<invoke_result_t<F, Args...>, R>::value> {};

template<typename F, typename... Args>
invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)
    noexcept(is_nothrow_invocable<F, Args...>::value) {
    return impl::INVOKE(std::forward<F>(f), std::forward<Args>(args)...);
}

#endif

} // namespace detail
} // namespace resilient