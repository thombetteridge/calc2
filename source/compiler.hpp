#pragma once

#include <string>
#include <variant>

#include "util.hpp"

enum class Op_Kind : u8 {
   None,
   Val,
   Word,
   Add,
   Sub,
   Mul,
   Div,
   Dup,
   Swap,
   Var,
   Eof,
};

struct Op_Code {
   Op_Kind kind;
   double  value {};
   std::string  text {};
};


using Value = std::variant<double>;
