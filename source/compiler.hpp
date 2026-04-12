#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "util.hpp"

enum class Op_Kind : uint8
{
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

struct Op_Code
{
  Op_Kind     kind;
  double      value {};
  std::string text {};
};

using Value = std::variant<double>;

using User_Words = std::unordered_map<std::string, std::vector<Op_Code>>;

struct Token;

void compile(
  std::vector<Token> const& tokens,
  std::vector<Op_Code>&     op_codes,
  User_Words&               user_words);