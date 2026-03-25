
#include "pch/pch_stdc++.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numbers>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <fmt/format.h>

#include "calc2.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "util.hpp"

/* declutter */

using std::optional;
using std::string;
using std::string_view;
using std::unordered_map;
using std::vector;

using User_Words = unordered_map<string, vector<Op_Code>>;

/* ===========  EVAL ============== */

struct Add_Vistor {
   auto operator()(double x, double y) -> optional<Value>
   {
      return y + x;
   }
};

struct Sub_Vistor {
   auto operator()(double x, double y) -> optional<Value>
   {
      return y - x;
   }
};

struct Mul_Vistor {
   auto operator()(double x, double y) -> optional<Value>
   {
      return y * x;
   }
};
struct Div_Vistor {
   auto operator()(double x, double y) -> optional<Value>
   {
      if (x == 0.0) return 0.0;
      return y / x;
   }
};

struct Stack {
   vector<Value> data;

   auto pop() -> optional<Value>
   {
      if (data.empty())
         return std::nullopt;

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
         return std::nullopt;

      return data.back();
   }

   auto pop2() -> optional<std::pair<Value, Value>>
   {
      if (data.size() < 2)
         return std::nullopt;

      auto const x = data.back();
      data.pop_back();
      auto const y = data.back();
      data.pop_back();
      return std::pair(x, y);
   }

   auto begin() { return data.begin(); }
   auto end() { return data.end(); }
};

struct Frame {
   unordered_map<string, Value> locals;
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

         case Op_Kind::None:
            Panic {}("Op_Kind NONE ??");
            break;

         case Op_Kind::Val:
            stack.push(op.value);
            break;
         case Op_Kind::Add: {
            auto const opt = stack.pop2();
            if (!opt) {
               Warning {}("Stack Underflow, Add");
               break;
            }

            auto const& [x, y] = *opt;
            auto const result  = std::visit(Add_Vistor {}, x, y);
            if (result)
               stack.push(*result);
            else
               Warning {}("Type error, Add");
            break;
         }
         case Op_Kind::Sub: {
            auto const opt = stack.pop2();
            if (!opt) {
               Warning {}("Stack Underflow, Sub");
               break;
            }

            auto const& [x, y] = *opt;
            auto const result  = std::visit(Sub_Vistor {}, x, y);
            if (result)
               stack.push(*result);
            else
               Warning {}("Type error, Sub");
            break;
         }
         case Op_Kind::Mul: {
            auto const opt = stack.pop2();
            if (!opt) {
               Warning {}("Stack Underflow, Mul");
               break;
            }

            auto const& [x, y] = *opt;
            auto const result  = std::visit(Mul_Vistor {}, x, y);
            if (result)
               stack.push(*result);
            else
               Warning {}("Type error, Mul");
            break;
         }
         case Op_Kind::Div: {
            auto const opt = stack.pop2();
            if (!opt) {
               Warning {}("Stack Underflow, Div");
               break;
            }

            auto const& [x, y] = *opt;
            auto const result  = std::visit(Div_Vistor {}, x, y);
            if (result)
               stack.push(*result);
            else
               Warning {}("Type error, Div");
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
               Warning {}("Stack Underflow, Swap");
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

            if (user_words.contains(op.text)) {
               eval(user_words[op.text]);
               break;
            }

            if (variables.contains(op.text)) {
               stack.push(variables[op.text]);
               break;
            }

            Warning {}("Unknown word: ", op.text);
            break;
         }
         case Op_Kind::Var: {

            auto split_string = [](std::string_view& s, char delim) -> std::string_view {
               auto const pos = s.find(delim);
               if (pos == std::string_view::npos) {
                  auto const result = s;
                  s                 = {};
                  return result;
               }

               auto const result = s.substr(0, pos);
               s.remove_prefix(pos + 1);
               return result;
            };

            string_view    vars_sv = op.text;
            vector<string> var_idents;

            while (!vars_sv.empty()) {
               string_view const ident = split_string(vars_sv, ' ');
               if (!ident.empty())
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

struct Map_Vistor {

   double (*fn)(double);

   auto operator()(double x) -> optional<Value>
   {
      return fn(x);
   }
};

// clang-format off
const Builtins Program::builtins = {
   {
    "sin", [](Program& P) {
      auto const opt = P.stack.pop();
      if (!opt) {
         Warning{} ("Stack underflow sin");
         return;
      }

      auto const result = std::visit(Map_Vistor{std::sin}, *opt);
      if (result) P.stack.push(*result);
      else Warning{} ("Type error sin");
     }
   },
   {
      "cos", [](Program& P) {
         auto const opt = P.stack.pop();
         if (!opt) {
            Warning{} ("Stack underflow cos");
            return;
         }

         auto const result = std::visit(Map_Vistor{std::cos}, *opt);
         if (result) P.stack.push(*result);
         else Warning{} ("Type error cos");
      }
   },
   {
      "tan", [](Program& P) {
         auto const opt = P.stack.pop();
         if (!opt) {
            Warning{} ("Stack underflow tan");
            return;
         }

         auto const result = std::visit(Map_Vistor{std::tan}, *opt);         
         if (result) P.stack.push(*result);
         else Warning{} ("Type error tan");
      }
   },
   {
      "sqrt", [](Program& P) {
         auto const opt = P.stack.pop();
         if (!opt) {
            Warning{} ("Stack underflow sqrt");
            return;
         }

         auto const result = std::visit(Map_Vistor{std::sqrt}, *opt);
         if (result) P.stack.push(*result);
         else Warning{} ("Type error sqrt");
      } 
   },
   {
      "pi", [](Program& P) {
         P.stack.push(std::numbers::pi);
      }
   },
};

static auto value_to_string(Value const& value) -> string
{
   return std::visit(overload {
      [](double x) {
         static constexpr double precision = 1e12;
         return fmt::format("{:.12g}", std::round(x*precision) / precision);
      },
      [] (auto v) {
         (void)v; UNREACHABLE() ;
      }
   }, value);
}
// clang-format on

auto run_calc(char const* input, size_t n) -> string
{

   static Program P {};

   P.clear_stack();

   Lexer lx { input, n };

   auto toks = lx.get_tokens();

   vector<Op_Code> codes;
   compile(toks, codes, P.user_words);
   P.eval(codes);

   std::string result {};
   for (auto const& value : P.stack) {
      result += value_to_string(value);
      result += '\n';
   }
   return result;
}