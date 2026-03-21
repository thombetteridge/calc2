
#include <algorithm>
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
#include <variant>
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
   Arrow,
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

static auto is_white(char c) -> bool
{
   return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static auto is_alpha(char c) -> bool
{
   return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static auto is_digit(char c) -> bool
{
   return c >= '0' && c <= '9';
}

static auto is_delim(char c) -> bool
{
   return c == ';' || c == ':' || c == '[' || c == ']';
}

static auto is_operator(char c) -> bool
{
   return c == '+' || c == '-' || c == '*' || c == '/';
}

static auto lx_new_number(Lexer& lx) -> Token
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

static auto lx_new_word(Lexer& lx) -> Token
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

static auto lx_new_token(Lexer& lx, Tok_Kind kind) -> Token
{
   Token tok = {
      .kind = kind,
      .text = string_view(lx.src.data() + lx.pos, 1),
   };
   lx.advance();
   return tok;
}

static auto lx_new_token2(Lexer& lx, Tok_Kind kind) -> Token
{
   Token tok = {
      .kind = kind,
      .text = string_view(lx.src.data() + lx.pos, 2),
   };
   lx.advance();
   lx.advance();
   return tok;
}

auto lx_next_token(Lexer& lx) -> Token
{
   while (is_white(lx.ch)) {
      lx.advance();
   }
   // clang-format off
   switch (lx.ch) {
   case '\0': return Token { .kind = Tok_Kind::Eof, .text = string_view("EOF") };
   case '+': return lx_new_token(lx, Tok_Kind::Plus);
   case '-':
      if (lx.pos+1 < lx.src.size() && lx.src[lx.pos+1] == '>') return lx_new_token2(lx, Tok_Kind::Arrow);
      else   return lx_new_token(lx, Tok_Kind::Minus);
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

void print_token(Token const& t)
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
   case Tok_Kind::Arrow:    writeln("Tok Kind: Arrow, Text: ",    t.text); break;
   }

   // clang-format on
}

auto src_to_tokens(char const* src) -> vector<Token>
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
   Var,
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

void compile_one(vector<Op_Code>& out, std::vector<Token> const& tokens, size_t& i)
{
   auto const& tok = tokens[i];
   switch (tok.kind) {
   case Tok_Kind::Number: {
      double     value {};
      auto const result = std::from_chars(
        tok.text.data(),
        tok.text.data() + tok.text.size(),
        value);

      if (result.ec != std::errc()) {
         // Panic {}("Malformed number: ", tok.text);
         break;
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

   case Tok_Kind::Arrow: {
      ++i;
      size_t       count = 0;
      size_t const start = i;
      while (i < tokens.size() && tokens[i].kind == Tok_Kind::Word && tokens[i].kind != Tok_Kind::Semi) {
         ++count;
         ++i;
      }

      if (tokens[i].kind != Tok_Kind::Semi) {
         break;
      }

      string idents {};
      for (size_t j = start; j < i + count; ++j) {
         idents += string(tokens[j].text) + ' ';
      }

      Op_Code op = { .kind = Op_Kind::Var, .value = static_cast<double>(count), .text = idents };

      out.push_back(op);
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
      // Panic {}("Unknown token: ", tok.text);
   case Tok_Kind::Colon:
      // Panic {}("unexpected ':'");
   case Tok_Kind::Semi:
      // Panic {}("unexpected ';'");
   case Tok_Kind::Eof:
      break;
   case Tok_Kind::LBracket:
      // Todo {}("LBracket");
   case Tok_Kind::RBracket:
      // Todo {}("RBracket");
      break;
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
            // Panic {}("missing word name after ':' ");
            break;
         }

         std::string name(tokens[i + 1].text);
         i += 2; /* skip "name :" */

         vector<Op_Code> body;
         while (i < tokens.size() && tokens[i].kind != Tok_Kind::Semi) {
            compile_one(body, tokens, i);
            ++i;
         }

         if (i >= tokens.size() || tokens[i].kind != Tok_Kind::Semi) {
            // Panic {}("missing ';' after definition of ", name);
            break;
         }

         user_words[name] = std::move(body);
         continue;
      }

      compile_one(op_codes, tokens, i);
   }
}
/* ===========  EVAL ============== */

using Value = std::variant<double>;

static auto as_number(Value const& v) -> optional<double>
{
   if (auto p = std::get_if<double>(&v))
      return *p;
   return nullopt;
}

static auto make_number(double x) -> Value
{
   return Value { x };
}

static auto add_values(Value const& a, Value const& b) -> optional<Value>
{
   auto xa = as_number(a);
   auto xb = as_number(b);
   if (!xa || !xb) return nullopt;
   return make_number(*xa + *xb);
}

static auto sub_values(Value const& a, Value const& b) -> optional<Value>
{
   auto xa = as_number(a);
   auto xb = as_number(b);
   if (!xa || !xb) return nullopt;
   return make_number(*xa - *xb);
}

static auto mul_values(Value const& a, Value const& b) -> optional<Value>
{
   auto xa = as_number(a);
   auto xb = as_number(b);
   if (!xa || !xb) return nullopt;
   return make_number(*xa * *xb);
}

static auto div_values(Value const& a, Value const& b) -> optional<Value>
{
   auto xa = as_number(a);
   auto xb = as_number(b);
   if (!xa || !xb) return nullopt;
   if (*xb == 0) return make_number(0); // or return nullopt/error later
   return make_number(*xa / *xb);
}

struct Stack {
   vector<Value> data;

   auto pop() -> optional<Value>
   {
      if (data.empty())
         return nullopt;

      auto const x = data.back();
      data.pop_back();
      return x;
   }

   void push(Value x)
   {
      data.push_back(x);
   }

   auto top() -> optional<Value>
   {
      if (data.empty())
         return nullopt;

      return data.back();
   }

   auto pop2() -> optional<std::pair<Value, Value>>
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
   Stack                        stack;
   User_Words                   user_words;
   unordered_map<string, Value> variables;
   static Builtins const        builtins;

   void eval(vector<Op_Code> const& codes)
   {
      for (auto const& op : codes) {
         switch (op.kind) {

         case Op_Kind::Val:
            stack.push(op.value);
            break;
         case Op_Kind::Add: {
            auto const opt = stack.pop2();
            if (!opt) {
               std::cerr << "Stack Underflow, Add\n";
               break;
            }

            auto const& [x, y] = *opt;
            auto result        = add_values(y, x);
            if (result)
               stack.push(*result);
            else
               std::cerr << "Type error, Add\n";
            break;
         }
         case Op_Kind::Sub: {
            auto const opt = stack.pop2();
            if (!opt) {
               std::cerr << "Stack Underflow, Sub\n";
               break;
            }

            auto const& [x, y] = *opt;
            auto result        = sub_values(y, x);
            if (result)
               stack.push(*result);
            else
               std::cerr << "Type error, Sub\n";
            break;
         }
         case Op_Kind::Mul: {
            auto const opt = stack.pop2();
            if (!opt) {
               std::cerr << "Stack Underflow, Mul\n";
               break;
            }

            auto const& [x, y] = *opt;
            auto result        = mul_values(y, x);
            if (result)
               stack.push(*result);
            else
               std::cerr << "Type error, Mul\n";
            break;
         }
         case Op_Kind::Div: {
            auto const opt = stack.pop2();
            if (!opt) {
               std::cerr << "Stack Underflow, Div\n";
               break;
            }

            auto const& [x, y] = *opt;

            if (as_number(y).value() == 0.0) {
               std::cerr << "Div by 0, Div\n";
               stack.push(make_number(0));
               break;
            }

            auto result = div_values(y, x);
            if (result)
               stack.push(*result);
            else
               std::cerr << "Type error, Div\n";
            break;

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

            if (variables.contains(op.text)) {
               stack.push(variables[op.text]);
            }

            break;
         }
         case Op_Kind::Var: {

            auto split_string = [](string_view& s, char delim) -> string_view {
               string_view result {};
               auto const  it = std::find(s.begin(), s.end(), delim);
               if (it != s.end()) {
                  result = string_view(s.begin(), it);
                  s      = string_view(it + 1, s.end());
               }
               return result;
            };

            string_view    vars_sv = op.text;
            vector<string> var_idents;

            while (!vars_sv.empty()) {
               string_view ident = split_string(vars_sv, ' ');
               var_idents.emplace_back(ident);
            }
            auto const num = static_cast<size_t>(op.value);
            for (size_t i = 0; i < std::min(num, var_idents.size()); ++i) {
               auto const opt = stack.pop();
               if (opt)
                  variables[var_idents[i]] = opt.value();
            }
            break;
         }
         case Op_Kind::Eof:
            return;
         }
      }
   }

   void clear_stack()
   {
      stack.data.clear();
   }
};

// clang-format off
const Builtins Program::builtins = {
   {
      "sin", [](Program& P) {
         auto const opt = P.stack.pop();
         if (opt)
            P.stack.push(std::sin(as_number(*opt).value()));
         else
            std::cerr << "Stack underflow sin\n";
      }
   },
   {
      "cos", [](Program& P) {
         auto const opt = P.stack.pop();
         if (opt)
            P.stack.push(as_number(*opt).value());
         else
            std::cerr << "Stack underflow cos\n";
      }
   },
   {
      "tan", [](Program& P) {
         auto const opt = P.stack.pop();
         if (opt)
            P.stack.push(as_number(*opt).value());
         else
            std::cerr << "Stack underflow tan\n";
      }
   },
   {
      "sqrt", [](Program& P) {
         auto const opt = P.stack.pop();
         if (opt)
            P.stack.push(as_number(*opt).value());
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

static auto value_to_string(Value const& v) -> string
{
   if (auto p = std::get_if<double>(&v))
      return std::format("{}", *p);
   return "<unknown>";
}

auto run_calc(char const* input) -> string
{

   static Program P {};

   P.clear_stack();

   auto toks = src_to_tokens(input);

   std::cout << "==Tokens==\n";

   for (auto t : toks) {
      print_token(t);
   }

   vector<Op_Code> codes;
   compile(toks, codes, P.user_words);
   P.eval(codes);

   std::string result;
   for (auto const& value : P.stack) {
      result += value_to_string(value);
      result += '\n';
   }
   return result;
}