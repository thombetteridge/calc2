#pragma once

#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <source_location>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct Panic {
  std::source_location loc;

  explicit Panic(std::source_location l = std::source_location::current())
      : loc(l) {}

  template <typename... Args>
  [[noreturn]] void operator()(Args &&...args) const
  {
    std::cerr << "Panic @ "
              << loc.file_name()
              << ":" << loc.line()
              << ": ";
    (std::cerr << ... << args);
    std::cerr << '\n';
    std::exit(1);
  }
};

struct Todo {
  std::source_location loc;

  explicit Todo(std::source_location l = std::source_location::current())
      : loc(l) {}

  template <typename... Args>
  [[noreturn]] void operator()(Args &&...args) const
  {
    std::cerr << "Panic @ "
              << loc.file_name()
              << ":" << loc.line()
              << ": TODO: ";
    (std::cerr << ... << args);
    std::cerr << '\n';
    std::exit(1);
  }
};

template <typename... Args>
[[noreturn]] void writeln(Args &&...args)
{
  (std::cout << ... << args);
  std::cout << '\n';
}
