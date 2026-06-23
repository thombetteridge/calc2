#pragma once

#include <string_view>
#include <vector>

#include "util.hpp"

enum class Tok_Kind : uint8
{
   None,
   Eof,
   Unknown,
   Number,
   Word,
   Plus,
   Minus,
   Slash,
   Star,
   Dot,
   Tilda,
   Colon,
   Semi,
   Arrow,

   LBracket,
   RBracket,
   LParen,
   RParen,
   LBrace,
   RBrace
};

struct Token
{
   Tok_Kind         kind { };
   std::string_view text { };
};

struct Lexer
{
   std::string_view src { };
   size_t           pos { };
   size_t           read_pos { };
   char             ch { };

   Lexer(char const* src, size_t n);

   void advance();
   auto get_tokens() -> std::vector<Token>;
};