#pragma once

#include <cstdio>
#include <cstdlib>

#include <print>
#include <source_location>
#include <utility>

#define let auto const

template <typename Out, typename In>
inline constexpr auto
as(In in) -> Out
{
    return static_cast<Out>(in);
}

template <typename Out, typename In>
inline auto
cast(In in) -> Out
{
    return reinterpret_cast<Out>(in);
}


namespace detail {
template <typename... Args>
void log(char const * color, char const * label, std::source_location loc, Args &&... args)
{
    std::print(stderr, "\033[{}m{} \033[0m{}:{}: \033[1m", color, label, loc.file_name(), loc.line());
    (std::print(stderr, "{}", std::forward<Args>(args)), ...);
    std::print(stderr, "\033[0m\n");
}
} // namespace detail

template <typename... Args>
struct panic {
    [[noreturn]] explicit panic(Args &&... args, std::source_location loc = std::source_location::current())
    {
        detail::log("31;1", "Panic", loc, std::forward<Args>(args)...);
        std::abort();
    }
};

template <typename... Args>
panic(Args &&...) -> panic<Args...>;


template <typename... Args>
struct trace {
    explicit trace(Args &&... args, std::source_location loc = std::source_location::current())
    {
        detail::log("32;1", "Trace", loc, std::forward<Args>(args)...);
    }
};

template <typename... Args>
trace(Args &&...) -> trace<Args...>;


template <typename F>
struct Cleanup {
    F cleanup_fn;
    Cleanup(F f) : cleanup_fn(f) {}
    ~Cleanup() { cleanup_fn(); }
};

#define STRCAT2(a, b) a##b
#define STRCAT(a, b)  STRCAT2(a, b)
#define cleanup(...)  Cleanup STRCAT(cleanup_, __LINE__) = [&] { __VA_ARGS__; }
