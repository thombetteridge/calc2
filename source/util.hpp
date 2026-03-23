#pragma once

#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <source_location>

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// https://en.cppreference.com/w/cpp/utility/unreachable.html
// use a macro until we get c++23
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
#define UNREACHABLE() __assume(false);
#else // GCC, Clang
#define UNREACHABLE() __builtin_unreachable();
#endif

struct Panic {
   std::source_location loc;

   explicit Panic(std::source_location l = std::source_location::current())
       : loc(l)
   { }

   template <typename... Args>
   [[noreturn]] void operator()(Args&&... args) const
   {
      std::cerr << "\033[31;1m"
                << "PANIC "
                << "\033[0m"
                << loc.file_name()
                << ":" << loc.line()
                << ": "
                << "\033[1m";
      if constexpr (sizeof...(args) > 0) {
         (std::cerr << ... << args);
      }
      std::cerr << "\033[0m" << "\n";
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
      std::cerr << "\033[31;1m"
                << "TODO "
                << "\033[0m"
                << loc.file_name()
                << ":" << loc.line()
                << ": "
                << "\033[1m";

      if constexpr (sizeof...(args) > 0) {
         (std::cerr << ... << args);
      }

      std::cerr << "\033[0m" << '\n';
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
      std::cerr << "\033[32;1m"
                << "TRACE "
                << "\033[0m"
                << loc.file_name()
                << ":" << loc.line()
                << ": "
                << "\033[1m";

      if constexpr (sizeof...(args) > 0) {
         (std::cerr << ... << args);
      }

      std::cerr << "\033[0m" << '\n';
   }
};

template <typename... Args>
[[maybe_unused]] void writeln(Args&&... args)
{
   if constexpr (sizeof...(args) > 0) {
      (std::cout << ... << args);
   }
   std::cout << '\n';
}

template <typename F>
struct Scoped {
   F f;
   Scoped(F f)
       : f(f)
   { }
   ~Scoped() { f(); }
};

#define STRING_CAT2(arg1, arg2) arg1##arg2
#define STRING_CAT(arg1, arg2)  STRING_CAT2(arg1, arg2)
#define scope_exit(code) \
   auto STRING_CAT2(scope_exit_, __LINE__) = Scoped([&]() -> auto { code; })