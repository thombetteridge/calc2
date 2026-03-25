#pragma once

#include <cstdint>
#include <cstdlib>

#include <source_location>

#include <fmt/format.h>

using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

// https://en.cppreference.com/w/cpp/utility/unreachable.html
// use a macro until we get c++23
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
#define UNREACHABLE() __assume(false);
#else // GCC, Clang
#define UNREACHABLE() __builtin_unreachable()
#endif

struct Panic {
   std::source_location loc;

   explicit Panic(std::source_location l = std::source_location::current())
       : loc(l)
   { }

   template <typename... Args>
   [[noreturn]] void operator()(Args&&... args) const
   {
      fmt::print("\033[31;1mPANIC \033[0m{}:{}: \033[1m", loc.file_name(), loc.line());
      if constexpr (sizeof...(args) > 0) {
         (fmt::print("{}", std::forward<Args>(args)), ...);
      }
      fmt::print("\033[0m\n");
      std::exit(1);
   }
};

struct Todo {
   std::source_location loc;

   explicit Todo(std::source_location l = std::source_location::current())
       : loc(l)
   { }

   template <typename... Args>
   [[noreturn]] void operator()(Args&&... args) const
   {
      fmt::print(stderr, "\033[31;1mTODO \033[0m{}:{}: \033[1m", loc.file_name(), loc.line());
      if constexpr (sizeof...(args) > 0) {
         (fmt::print(stderr, "{}", std::forward<Args>(args)), ...);
      }
      fmt::print(stderr, "\033[0m\n");
      std::exit(1);
   }
};

struct Log {
   std::source_location loc;

   explicit Log(std::source_location l = std::source_location::current())
       : loc(l)
   { }

   template <typename... Args>
   [[maybe_unused]] void operator()(Args&&... args) const
   {
      fmt::print(stderr, "\033[32;1mLOG \033[0m{}:{}: \033[1m", loc.file_name(), loc.line());
      if constexpr (sizeof...(args) > 0) {
         (fmt::print(stderr, "{}", std::forward<Args>(args)), ...);
      }
      fmt::print(stderr, "\033[0m\n");
   }
};

struct Warning {
   std::source_location loc;

   explicit Warning(std::source_location l = std::source_location::current())
       : loc(l)
   { }

   template <typename... Args>
   [[maybe_unused]] void operator()(Args&&... args) const
   {
      fmt::print(stderr, "\033[33;1mWARNING \033[0m{}:{}: \033[1m", loc.file_name(), loc.line());
      if constexpr (sizeof...(args) > 0) {
         (fmt::print(stderr, "{}", std::forward<Args>(args)), ...);
      }
      fmt::print(stderr, "\033[0m\n");
   }
};

template <typename... Args>
[[maybe_unused]] void writeln(Args&&... args)
{
   if constexpr (sizeof...(args) > 0) {
      (fmt::print("{}", std::forward<Args>(args)), ...);
   }
   fmt::print("\n");
}

template <class... Ts>
struct overload : Ts... {
   using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

template <typename F>
struct Scoped {
   F cleanup;
   explicit Scoped(F f) noexcept
       : cleanup(f)
   { }
   ~Scoped() noexcept { std::move(cleanup); }
   Scoped(Scoped const&)         = delete;
   Scoped(Scoped&&)              = delete;
   auto operator=(Scoped const&) = delete;
   auto operator=(Scoped&&)      = delete;
};

template <class F>
Scoped(F) -> Scoped<F>;

#define STRING_CAT2(arg1, arg2) arg1##arg2
#define STRING_CAT(arg1, arg2)  STRING_CAT2(arg1, arg2)
#define scope_exit(code)                           \
   auto STRING_CAT(scope_exit_, __LINE__) = Scoped \
   {                                               \
      [&]() -> auto { code; }                      \
   }
