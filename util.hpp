#pragma once

#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <source_location>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

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
      std::cerr << "Panic @ "
                << loc.file_name()
                << ":" << loc.line()
                << ": ";
      if constexpr (sizeof...(args) > 0) {
         (std::cerr << ... << args);
      }
      std::cerr << '\n';
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
      std::cerr << "Todo @ "
                << loc.file_name()
                << ":" << loc.line()
                << ": TODO: ";

      if constexpr (sizeof...(args) > 0) {
         (std::cerr << ... << args);
      }

      std::cerr << '\n';
      std::exit(1);
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
