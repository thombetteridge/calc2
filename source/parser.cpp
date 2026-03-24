
#if 0

#include "util.hpp"
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using std::optional;
using std::string;
using std::string_view;
using std::vector;

#include "compiler.hpp"
#include "lexer.hpp"

struct Parse_Error {
   bool             error {};
   std::string_view text {};

   Parse_Error(char const* msg)
       : error(true)
       , text(msg)
   { }
   Parse_Error() = default;

   operator bool() const
   {
      return error;
   }
};

auto parse_primary(vector<Op_Code>& out, vector<Token> const& tokens, size_t& i) -> Parse_Error;

static auto sv_to_double(string_view const sv) -> optional<double>
{
   double     value {};
   auto const result = std::from_chars(
     sv.data(),
     sv.data() + sv.size(),
     value);

   if (result.ec != std::errc()) {
      return std::nullopt;
   }
   return value;
}

static auto is_right_assoc(Token const& t) -> bool
{
    switch (t.kind) {
        default:
        	Warning{}("not implemented right assoc");
            return false;
    }
}

static auto precedence(Token const& t) -> int
{
    switch (t.kind) {
        case Tok_Kind::Star:
        case Tok_Kind::Slash:
            return 20;

        case Tok_Kind::Plus:
        case Tok_Kind::Minus:
            return 10;

        default:
            return -1;
    }
}

static auto is_binary_op(Token const& t) -> bool {
	switch(t.kind) {
	case Tok_Kind::Plus:
	case Tok_Kind::Minus:
	case Tok_Kind::Slash:
	case Tok_Kind::Star:
		return true;
	default:
		return false;
	}
}

static auto parse_expr(vector<Op_Code>& out, vector<Token> const& tokens, size_t& i, int min_prec) -> Parse_Error
{
   {
      auto const err = parse_primary(out, tokens, i);
      if (err) return err;
   }

   while (i < tokens.size() && is_binary_op(tokens[i])) {
      auto const& op   = tokens[i];
      int const   prec = precedence(op);

      if(prec < min_prec) break;

      bool const r_assoc = is_right_assoc(op);

      ++i; // consume op;

      int const next_min_prec = r_assoc ? prec : prec + 1;
      parse_expr(out, tokens, i, next_min_prec);

      out.push_back(token_to_opcode(op));
   }
}

auto parse_primary(vector<Op_Code>& out, vector<Token> const& tokens, size_t& i) -> Parse_Error
{
   if (i >= tokens.size()) {
      return { "unexpected end of input" };
   }

   auto const& tok = tokens[i];

   if (tok.kind == Tok_Kind::Number) {
      auto const opt = sv_to_double(tok.text);
      if (opt)
         out.push_back({ .kind = Op_Kind::Val, .value = opt.value() });
      else
         Warning {}("Malformed Number");
      return {};
   }

   if (tok.kind == Tok_Kind::Word) {
      out.push_back({ .kind = Op_Kind::Word, .text = string(tok.text) });
      return {};
   }

   if (tok.kind == Tok_Kind::LParen) {
      ++i;
      auto const err = parse_expr(out, tokens, i, 0);
      if (err) {
         return err;
      }
      if (i >= tokens.size() || tokens[i].kind != Tok_Kind::RParen) {
         return { "expected ')'" };
      }

      ++i;
      return {};
   }

   return { "expected expression" };
}

void parse_infix(vector<Op_Code>& out, vector<Token> const& tokens, size_t& i)
{
   vector<Op_Code> ops {};

   ++i; // skip '('

   while (i < tokens.size() && tokens[i].kind != Tok_Kind::RParen) {

      ++i;
   }

   if (i < tokens.size() && tokens[i].kind == Tok_Kind::RParen)
      ++i; // skip ')'

   out.insert(out.end(), ops.begin(), ops.end());
}

#endif