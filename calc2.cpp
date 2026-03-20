
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <format>
#include <iostream>
#include <numbers>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "util.hpp"

/* declutter */

using std::optional;
using std::string;
using std::string_view;
using std::unordered_map;
using std::vector;

using namespace std;

/* ============= LEXER =========== */

enum class Tok_Kind {
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
   LBracket,
   RBracket
};

struct Token {
   Tok_Kind    kind;
   string_view text;
};

struct Lexer {
   string_view src;
   size_t      pos;
   size_t      read_pos;
   char        ch;

   void advance()
   {
      if (read_pos > src.size()) {
         ch = '\000';
         return;
      }
      pos = read_pos;
      ch  = src[pos];
      ++read_pos;
   }
};

static bool is_white(char c)
{
   return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static bool is_alpha(char c)
{
   return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_digit(char c)
{
   return c >= '0' && c <= '9';
}

static bool is_delim(char c)
{
   return c == ';' || c == ':' || c == '[' || c == ']';
}

static bool is_operator(char c)
{
   return c == '+' || c == '-' || c == '*' || c == '/';
}

static Token lx_new_number(Lexer& lx)
{
   size_t const start = lx.pos;

   while (is_digit(lx.ch) || lx.ch == '.') {
      lx.advance();
   }

   return Token {
      .kind = Tok_Kind::Number,
      .text = string_view(lx.src.data() + start, lx.pos - start),
   };
}

static Token lx_new_word(Lexer& lx)
{
   size_t const start = lx.pos;

   while (!(is_white(lx.ch) || is_delim(lx.ch) || is_operator(lx.ch) || lx.ch == '\000')) {
      lx.advance();
   }

   return Token {
      .kind = Tok_Kind::Word,
      .text = string_view(lx.src.data() + start, lx.pos - start),
   };
}

static Token lx_new_token(Lexer& lx, Tok_Kind kind)
{
   Token tok = {
      .kind = kind,
      .text = string_view(lx.src.data() + lx.pos, 1),
   };
   lx.advance();
   return tok;
}

Token lx_next_token(Lexer& lx)
{
   while (is_white(lx.ch)) {
      lx.advance();
   }
   // clang-format off
   switch (lx.ch) {
   case '\0': return Token { .kind = Tok_Kind::Eof, .text = string_view("EOF") };
   case '+': return lx_new_token(lx, Tok_Kind::Plus);
   case '-': return lx_new_token(lx, Tok_Kind::Minus);
   case '/': return lx_new_token(lx, Tok_Kind::Slash);
   case '*': return lx_new_token(lx, Tok_Kind::Star);
   case ':': return lx_new_token(lx, Tok_Kind::Colon);
   case ';': return lx_new_token(lx, Tok_Kind::Semi);
   case '~': return lx_new_token(lx, Tok_Kind::Tilda);
   case '[': return lx_new_token(lx, Tok_Kind::LBracket);
   case ']': return lx_new_token(lx, Tok_Kind::RBracket);
   case '.' : {
      if (lx.pos + 1 < lx.src.size() && is_digit (lx.src[lx.pos+1]) )
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

static void print_token(Token const& t)
{
   // clang-format off
   switch (t.kind) {
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
   case Tok_Kind::LBracket: writeln("Tok Kind: LBracket, Text: ", t.text); break;
   case Tok_Kind::RBracket: writeln("Tok Kind: RBracket, Text: ", t.text); break;
   case Tok_Kind::Dot:      writeln("Tok Kind: Dot, Text: ",      t.text); break;
   case Tok_Kind::Tilda:    writeln("Tok Kind: Tilda, Text: ",    t.text); break;
   }

   // clang-format on
}

vector<Token> src_to_tokens(char const* src)
{
   vector<Token> result;

   Lexer lx = {
      .src      = src,
      .pos      = 0,
      .read_pos = 0,
      .ch       = '\000',
   };

   lx.advance();

   Token tok;
   do {
      tok = lx_next_token(lx);
      result.push_back(tok);
   } while (tok.kind != Tok_Kind::Eof);

   return result;
}

/* ============= COMPILE =========== */

enum class Op_Kind {
   Val,
   Word,
   Add,
   Sub,
   Mul,
   Div,
   Dup,
   Swap,
   Eof,
};

struct Op_Code {
   Op_Kind kind;
   double  value {};
   string  text {};
};

using User_Words = unordered_map<string, vector<Op_Code>>;

unordered_map<string_view, Op_Code> const intrinsics = {
   { "dup", Op_Code { .kind = Op_Kind::Dup } },
   { "swap", Op_Code { .kind = Op_Kind::Swap } },
};

void compile_one(vector<Op_Code>& out, Token const& tok)
{
   switch (tok.kind) {
   case Tok_Kind::Number: {
      double     value;
      auto const result = std::from_chars(
        tok.text.data(),
        tok.text.data() + tok.text.size(),
        value);

      if (result.ec != std::errc()) {
         Panic {}("Malformed number: ", tok.text);
      }

      out.push_back({ .kind = Op_Kind::Val, .value = value });
      break;
   }

   case Tok_Kind::Word:
      // we have to do it like this beacuse intrinsics is const
      {
         auto const it = intrinsics.find(tok.text);
         if (it != intrinsics.end()) {
            out.push_back(it->second);
            break;
         }
         else {
            out.push_back({ .kind = Op_Kind::Word, .text = string(tok.text) });
         }
         break;
      }

   case Tok_Kind::Plus:
      out.push_back({ .kind = Op_Kind::Add });
      break;
   case Tok_Kind::Minus:
      out.push_back({ .kind = Op_Kind::Sub });
      break;
   case Tok_Kind::Slash:
      out.push_back({ .kind = Op_Kind::Div });
      break;
   case Tok_Kind::Star:
      out.push_back({ .kind = Op_Kind::Mul });
      break;
   case Tok_Kind::Tilda:
      out.push_back({ .kind = Op_Kind::Swap });
      break;
   case Tok_Kind::Dot:
      out.push_back({ .kind = Op_Kind::Dup });
      break;
   case Tok_Kind::Unknown:
      Panic {}("Unknown token: ", tok.text);
   case Tok_Kind::Colon:
      Panic {}("unexpected ':'");
   case Tok_Kind::Semi:
      Panic {}("unexpected ';'");
   case Tok_Kind::Eof:
      break;
   case Tok_Kind::LBracket:
      Todo {}("LBracket");
   case Tok_Kind::RBracket:
      Todo {}("RBracket");
   }
}

void compile(
  vector<Token> const& tokens,
  vector<Op_Code>&     op_codes,
  User_Words&          user_words)
{
   for (size_t i = 0; i < tokens.size(); ++i) {
      auto const& tok = tokens[i];

      if (tok.kind == Tok_Kind::Eof) {
         break;
      }

      /* new user words */
      if (tok.kind == Tok_Kind::Colon) {
         if (i + 1 < tokens.size() && tokens[i + 1].kind != Tok_Kind::Word) {
            Panic {}("missing word name after ':' ");
         }

         std::string name(tokens[i + 1].text);
         i += 2; /* skip "name :" */

         vector<Op_Code> body;
         while (i < tokens.size() && tokens[i].kind != Tok_Kind::Semi) {
            compile_one(body, tokens[i]);
            ++i;
         }

         if (i >= tokens.size() || tokens[i].kind != Tok_Kind::Semi) {
            Panic {}("missing ';' after definition of ", name);
         }

         user_words[name] = std::move(body);
         continue;
      }

      compile_one(op_codes, tok);
   }
}
/* ===========  EVAL ============== */

struct Stack {
   vector<double> data;

   auto pop() -> optional<double>
   {
      if (data.empty())
         return nullopt;

      auto const x = data.back();
      data.pop_back();
      return x;
   }

   void push(double x)
   {
      data.push_back(x);
   }

   auto top() -> optional<double>
   {
      if (data.empty())
         return nullopt;

      return data.back();
   }

   auto pop2() -> optional<std::pair<double, double>>
   {
      if (data.size() < 2)
         return nullopt;

      auto const x = data.back();
      data.pop_back();
      auto const y = data.back();
      data.pop_back();
      return std::pair(x, y);
   }

   auto begin() { return data.begin(); }
   auto end() { return data.end(); }
};


struct Program;
using BuiltinFn = void (*)(Program&);
using Builtins  = std::unordered_map<std::string, BuiltinFn>;

struct Program {
   Stack                 stack;
   User_Words            user_words;
   static Builtins const builtins;

   string run(char const* input)
   {
      auto toks = src_to_tokens(input);

      std::cout << "==Tokens==\n";

      for (auto t : toks) {
         print_token(t);
      }

      vector<Op_Code> codes;
      compile(toks, codes, user_words);
      eval(codes);

      std::string result;
      for (auto const value : stack) {
         result += std::format("{}\n", value);
      }
      return result;
   }

   void eval(vector<Op_Code> codes)
   {
      for (auto const& op : codes) {
         switch (op.kind) {

         case Op_Kind::Val:
            stack.push(op.value);
            break;
         case Op_Kind::Add: {
            auto const opt = stack.pop2();
            if (opt) {
               auto const [x, y] = opt.value();
               stack.push(y + x);
            }
            else {
               std::cerr << "Stack Underflow, Add\n";
            }
            break;
         }
         case Op_Kind::Sub: {
            auto const opt = stack.pop2();
            if (opt) {
               auto const [x, y] = opt.value();
               stack.push(y - x);
            }
            else {
               std::cerr << "Stack Underflow, Sub\n";
            }
            break;
         }
         case Op_Kind::Mul: {
            auto const opt = stack.pop2();
            if (opt) {
               auto const [x, y] = opt.value();
               stack.push(y * x);
            }
            else {
               std::cerr << "Stack Underflow, Mul\n";
            }
            break;
         }
         case Op_Kind::Div: {
            auto const opt = stack.pop2();
            if (opt) {
               auto const [x, y] = opt.value();
               if (x == 0) {
                  stack.push(0);
               }
               else {
                  stack.push(y / x);
               }
            }
            else {
               std::cerr << "Stack Underflow, Div\n";
            }
         } break;
         case Op_Kind::Dup: {
            auto const opt = stack.top();
            if (opt) {
               stack.push(opt.value());
            }
            break;
         }
         case Op_Kind::Swap: {
            auto const opt = stack.pop2();
            if (opt) {
               auto const [x, y] = opt.value();
               stack.push(x);
               stack.push(y);
            }
            else {
               std::cerr << "Stack Underflow, Swap\n";
            }
            break;
         }
         case Op_Kind::Word: {

            // we have to do it like this beacuse it's const
            auto const it = builtins.find(op.text);
            if (it != builtins.end()) {
               it->second(*this);
               break;
            }

            if (user_words.contains(op.text))
               eval(user_words[op.text]);

            break;
         }
         case Op_Kind::Eof:
            return;
         }
      }
   }
};

// clang-format off
const Builtins Program::builtins = {
   {
      "sin", [](Program& P) {
         auto const opt = P.stack.pop();
         if (opt)
            P.stack.push(std::sin(*opt));
         else
            std::cerr << "Stack underflow sin\n";
      }
   },
   {
      "cos", [](Program& P) {
         auto const opt = P.stack.pop();
         if (opt)
            P.stack.push(std::cos(*opt));
         else
            std::cerr << "Stack underflow cos\n";
      }
   },
   {
      "tan", [](Program& P) {
         auto const opt = P.stack.pop();
         if (opt)
            P.stack.push(std::tan(*opt));
         else
            std::cerr << "Stack underflow tan\n";
      }
   },
   {
      "sqrt", [](Program& P) {
         auto const opt = P.stack.pop();
         if (opt)
            P.stack.push(std::sqrt(*opt));
         else
            std::cerr << "Stack underflow sqrt\n";
      }
   },
   {
      "pi", [](Program& P) {
         P.stack.push(std::numbers::pi);
      }
   },
};

// clang-format on

int main()
{
   Program prog;

   constexpr auto input = ":sq . *;"
                          ": hypot sq ~ sq + sqrt ;"
                          "3 4 hypot ";

   std::cout << prog.run(input);
}