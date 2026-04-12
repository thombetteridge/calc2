#include "lexer.hpp"

#include <string_view>
#include <vector>

using std::string_view;
using std::vector;

#define Local_Function [[nodiscard]] constexpr static auto

/* ============= LEXER =========== */

[[nodiscard]] constexpr static auto
is_white(char c) -> bool
{
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

[[nodiscard]] constexpr static auto
is_alpha(char c) -> bool
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

[[nodiscard]] constexpr static auto
is_digit(char c) -> bool
{
  return c >= '0' && c <= '9';
}

[[nodiscard]] constexpr static auto
is_delim(char c) -> bool
{
  return c == ';' || c == ':' || c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}';
}

[[nodiscard]] constexpr static auto
is_operator(char c) -> bool
{
  return c == '+' || c == '-' || c == '*' || c == '/';
}

[[nodiscard]] constexpr static auto
lx_new_number(Lexer& lx) -> Token
{
  size_t const start = lx.pos;

  if (lx.ch == '-')
    lx.advance();

  while (is_digit(lx.ch) || lx.ch == '.' || lx.ch == 'e')
    lx.advance();

  return Token {
    .kind = Tok_Kind::Number,
    .text = string_view(lx.src.substr(start, std::max(lx.pos - start, static_cast<size_t>(1)))),
  };
}

[[nodiscard]] constexpr static auto
lx_new_word(Lexer& lx) -> Token
{
  size_t const start = lx.pos;

  // Boolean expression can be simplified by DeMorgan's theorem (fix available) (clang-tidy)
  while (!is_white(lx.ch) && !is_delim(lx.ch) && !is_operator(lx.ch) && lx.ch != '\000') {
    lx.advance();
  }

  return Token {
    .kind = Tok_Kind::Word,
    .text = string_view(lx.src.substr(start, std::max(lx.pos - start, static_cast<size_t>(1)))),
  };
}

[[nodiscard]] static auto
lx_new_token(Lexer& lx, Tok_Kind kind) -> Token
{
  Token tok = {
    .kind = kind,
    .text = string_view(lx.src.substr(lx.pos, 1)),
  };
  lx.advance();
  return tok;
}

[[nodiscard]] static auto
lx_new_token2(Lexer& lx, Tok_Kind kind) -> Token
{
  Token tok = {
    .kind = kind,
    .text = string_view(lx.src.substr(lx.pos, 2)),
  };
  lx.advance();
  lx.advance();
  return tok;
}

[[nodiscard]] constexpr static auto
lx_peek(Lexer& lx) -> char
{
  if (lx.pos + 1 < lx.src.size()) {
    return lx.src[lx.pos + 1];
  }
  return char {};
}

[[nodiscard]] constexpr static auto
lx_next_token(Lexer& lx) -> Token
{
  while (is_white(lx.ch)) {
    lx.advance();
  }
  // clang-format off
   switch (lx.ch) {
   case '\0': return Token { .kind = Tok_Kind::Eof, .text = string_view("EOF") };
   case '+': return lx_new_token(lx, Tok_Kind::Plus);
   case '-':
      if (lx_peek(lx) == '>')
         return lx_new_token2(lx, Tok_Kind::Arrow);
      else if (is_digit (lx_peek(lx)))
         return lx_new_number(lx);
      else
         return lx_new_token(lx, Tok_Kind::Minus);
   case '/': return lx_new_token(lx, Tok_Kind::Slash);
   case '*': return lx_new_token(lx, Tok_Kind::Star);
   case ':': return lx_new_token(lx, Tok_Kind::Colon);
   case ';': return lx_new_token(lx, Tok_Kind::Semi);
   case '~': return lx_new_token(lx, Tok_Kind::Tilda);
   case '[': return lx_new_token(lx, Tok_Kind::LBracket);
   case ']': return lx_new_token(lx, Tok_Kind::RBracket);
   case '(': return lx_new_token(lx, Tok_Kind::LParen);
   case ')': return lx_new_token(lx, Tok_Kind::RParen);
   case '{': return lx_new_token(lx, Tok_Kind::LBrace);
   case '}': return lx_new_token(lx, Tok_Kind::RBrace);
   case '.' : {
      if (is_digit (lx_peek(lx)))
         return lx_new_number(lx);
      else
         return lx_new_token(lx, Tok_Kind::Dot);
   }
    // clang-format on
  default:
    if (is_digit(lx.ch))
      return lx_new_number(lx);
    else if (is_alpha(lx.ch))
      return lx_new_word(lx);
    else
      return lx_new_token(lx, Tok_Kind::Unknown);
  }
  UNREACHABLE();
}

// clang-format off
static void print_token(Token const& t)
{
   switch (t.kind) {
   case Tok_Kind::None:     writeln("Tok Kind: None, Text: ",     t.text); break;
   case Tok_Kind::Eof:      writeln("Tok Kind: Eof, Text: ",      t.text); break;
   case Tok_Kind::Unknown:  writeln("Tok Kind: Unknown, Text: ",  t.text); break;
   case Tok_Kind::Number:   writeln("Tok Kind: Number, Text: ",   t.text); break;
   case Tok_Kind::Word:     writeln("Tok Kind: Word, Text: ",     t.text); break;
   case Tok_Kind::Plus:     writeln("Tok Kind: Plus, Text: ",     t.text); break;
   case Tok_Kind::Minus:    writeln("Tok Kind: Minus, Text: ",    t.text); break;
   case Tok_Kind::Slash:    writeln("Tok Kind: Slash, Text: ",    t.text); break;
   case Tok_Kind::Star:     writeln("Tok Kind: Star, Text: ",     t.text); break;
   case Tok_Kind::Colon:    writeln("Tok Kind: Colon, Text: ",    t.text); break;
   case Tok_Kind::Semi:     writeln("Tok Kind: Semi, Text: ",     t.text); break;
   case Tok_Kind::Dot:      writeln("Tok Kind: Dot, Text: ",      t.text); break;
   case Tok_Kind::Tilda:    writeln("Tok Kind: Tilda, Text: ",    t.text); break;
   case Tok_Kind::Arrow:    writeln("Tok Kind: Arrow, Text: ",    t.text); break;
   case Tok_Kind::LBracket: writeln("Tok Kind: LBracket, Text: ", t.text); break;
   case Tok_Kind::RBracket: writeln("Tok Kind: RBracket, Text: ", t.text); break;
   case Tok_Kind::LParen:   writeln("Tok Kind: LParen, Text: ",   t.text); break;
   case Tok_Kind::RParen:   writeln("Tok Kind: RParen, Text: ",   t.text); break;
   case Tok_Kind::LBrace:   writeln("Tok Kind: LBrace, Text: ",   t.text); break;
   case Tok_Kind::RBrace:   writeln("Tok Kind: RBrace, Text: ",   t.text); break;
   }

  // clang-format on
}

//======================================//

Lexer::Lexer(char const* src, size_t n)
    : src(string_view(src, n))
{ }

void Lexer::advance()
{
  if (read_pos >= src.size()) {
    ch = '\000';
    return;
  }
  pos = read_pos;
  ch  = src[pos];
  ++read_pos;
}

auto Lexer::get_tokens() -> vector<Token>
{
  vector<Token> result;

  this->advance();

  Token tok;
  do {
    tok = lx_next_token(*this);
    result.push_back(tok);
  } while (tok.kind != Tok_Kind::Eof);

  fmt::print("=====Tokens=====\n");

  for (auto const& t : result) {
    print_token(t);
  }

  return result;
}