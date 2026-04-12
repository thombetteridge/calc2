#include "pch/pch_stdc++.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using std::optional;
using std::string;
using std::string_view;
using std::unordered_map;
using std::vector;

#include "compiler.hpp"
#include "lexer.hpp"
#include "util.hpp"

struct Parse_Error
{
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

auto parse_primary(vector<Op_Code>& out, vector<Token> const& tokens, size_t& index) -> Parse_Error;

unordered_map<string_view, Op_Code> const intrinsics = {
  { "dup", Op_Code { .kind = Op_Kind::Dup } },
  { "swap", Op_Code { .kind = Op_Kind::Swap } },
};

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

void parse_infix(vector<Op_Code>& out, vector<Token> const& tokens, size_t& index);

void compile_one(vector<Op_Code>& out, vector<Token> const& tokens, size_t& index)
{
  auto const& tok = tokens[index];
  switch (tok.kind) {
  case Tok_Kind::None:
    Panic {}("Tok None");

  case Tok_Kind::Number: {
    auto const opt = sv_to_double(tok.text);
    if (opt) {
      out.emplace_back(Op_Code { .kind = Op_Kind::Val, .value = opt.value() });
      ++index;
    }
    else
      Warning {}("Malformed number: ", tok.text);
    break;
  }

  case Tok_Kind::Word:
    // we have to do it like this beacuse intrinsics is const
    {
      auto const it = intrinsics.find(tok.text);
      if (it != intrinsics.end()) {
        out.emplace_back(it->second);
        ++index;
        break;
      }
      else {
        out.emplace_back(Op_Code { .kind = Op_Kind::Word, .text = string(tok.text) });
        ++index;
      }
      break;
    }

  case Tok_Kind::Arrow: {
    ++index; // consume arrow
    size_t       count = 0;
    size_t const start = index;
    while (index < tokens.size() && tokens[index].kind == Tok_Kind::Word && tokens[index].kind != Tok_Kind::Semi) {
      ++count;
      ++index;
    }

    if (tokens[index].kind != Tok_Kind::Semi) {
      Warning {}("Missing ; for ->");
      ++index;
      break;
    }

    string idents {};
    for (size_t j = start; j < index; ++j) {
      idents += string(tokens[j].text) + ' ';
    }

    out.emplace_back(Op_Code { .kind = Op_Kind::Var, .value = static_cast<double>(count), .text = idents });
    ++index; // consume the ';'
    break;
  }
  case Tok_Kind::Plus:
    out.emplace_back(Op_Code { .kind = Op_Kind::Add });
    ++index;
    break;
  case Tok_Kind::Minus:
    out.emplace_back(Op_Code { .kind = Op_Kind::Sub });
    ++index;
    break;
  case Tok_Kind::Slash:
    out.emplace_back(Op_Code { .kind = Op_Kind::Div });
    ++index;
    break;
  case Tok_Kind::Star:
    out.emplace_back(Op_Code { .kind = Op_Kind::Mul });
    ++index;
    break;
  case Tok_Kind::Tilda:
    out.emplace_back(Op_Code { .kind = Op_Kind::Swap });
    ++index;
    break;
  case Tok_Kind::Dot:
    out.emplace_back(Op_Code { .kind = Op_Kind::Dup });
    ++index;
    break;
  case Tok_Kind::Unknown:
    Warning {}("Unknown token: ", tok.text);
    ++index;
    break;
  case Tok_Kind::Colon:
    Warning {}("unexpected ':'");
    ++index;
    break;
  case Tok_Kind::Semi:
    Warning {}("unexpected ';'");
    ++index;
    break;
  case Tok_Kind::LBracket:
    Warning {}("TODO: LBracket");
    ++index;
    break;
  case Tok_Kind::RBracket:
    Warning {}("TODO: RParen");
    ++index;
    break;
  case Tok_Kind::LParen:
    parse_infix(out, tokens, index);
    break;
  case Tok_Kind::RParen:
    Warning {}("unexpected ')' ");
    ++index;
    break;
  case Tok_Kind::LBrace:
    Warning {}("TODO: LBrace");
    ++index;
    break;
  case Tok_Kind::RBrace:
    Warning {}("TODO: RBrace");
    ++index;
    break;
  case Tok_Kind::Eof:
    ++index;
    break;
  }
}

void print_ops(Op_Code const& op)
{
  switch (op.kind) {

  case Op_Kind::None:
    writeln("Op: None");
    break;
  case Op_Kind::Val:
    writeln("Op: Val, Value: ", op.value);
    break;
  case Op_Kind::Word:
    writeln("Op: Word, Text: ", op.text);
    break;
  case Op_Kind::Add:
    writeln("Op: Add");
    break;
  case Op_Kind::Sub:
    writeln("Op: Sub");
    break;
  case Op_Kind::Mul:
    writeln("Op: Mul");
    break;
  case Op_Kind::Div:
    writeln("Op: Div");
    break;
  case Op_Kind::Dup:
    writeln("Op: Dup");
    break;
  case Op_Kind::Swap:
    writeln("Op: Swap");
    break;
  case Op_Kind::Var:
    writeln("Op: Var Value: ", op.value, " Text: ", op.text);
    break;
  case Op_Kind::Eof:
    writeln("Op: Eof");
    break;
    break;
  }
}

void compile(
  vector<Token> const& tokens,
  vector<Op_Code>&     op_codes,
  User_Words&          user_words)
{
  size_t index = 0;
  while (index < tokens.size()) {
    auto const& tok = tokens[index];

    if (tok.kind == Tok_Kind::Eof) {
      break;
    }

    /* new user words */
    if (tok.kind == Tok_Kind::Colon) {

      if (index + 1 < tokens.size() && tokens[index + 1].kind != Tok_Kind::Word) {
        Warning {}("missing word name after ':' ");
        break;
      }

      ++index; // consume :
      auto const name = string { tokens[index].text };
      ++index; // consume name;

      vector<Op_Code> body;

      while (index < tokens.size() && tokens[index].kind != Tok_Kind::Semi) {
        compile_one(body, tokens, index);
      }

      if (index >= tokens.size() || tokens[index].kind != Tok_Kind::Semi) {
        Warning {}("missing ';' after definition of", name);
        break;
      }

      user_words[name] = std::move(body);
      ++index;
      continue;
    }

    compile_one(op_codes, tokens, index);
  }

  writeln("=== OP CODES ===");
  for (auto const& o : op_codes) {
    print_ops(o);
  }
}

static auto is_right_assoc(Token const& t) -> bool
{
  (void)t;
  return false;
  // TODO
  // switch (t.kind) {
  // default:
  //    Warning {}("not implemented right assoc");
  //
  //    return false;
  // }
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

static auto is_binary_op(Token const& t) -> bool
{
  switch (t.kind) {
  case Tok_Kind::Plus:
  case Tok_Kind::Minus:
  case Tok_Kind::Slash:
  case Tok_Kind::Star:
    return true;
  default:
    return false;
  }
}

static auto emit_binary_op(vector<Op_Code>& out, Token const& tok) -> Parse_Error
{
  switch (tok.kind) {
  case Tok_Kind::Plus:
    out.emplace_back(Op_Code { .kind = Op_Kind::Add });
    return {};
  case Tok_Kind::Minus:
    out.emplace_back(Op_Code { .kind = Op_Kind::Sub });
    return {};
  case Tok_Kind::Star:
    out.emplace_back(Op_Code { .kind = Op_Kind::Mul });
    return {};
  case Tok_Kind::Slash:
    out.emplace_back(Op_Code { .kind = Op_Kind::Div });
    return {};
  default:
    return { "expected binary operator" };
  }
}

static auto parse_expr(vector<Op_Code>& out, vector<Token> const& tokens, size_t& index, int min_prec) -> Parse_Error
{
  {
    auto const err = parse_primary(out, tokens, index);
    if (err) return err;
  }

  while (index < tokens.size() && is_binary_op(tokens[index])) {
    auto const op   = tokens[index];
    int const  prec = precedence(op);

    if (prec < min_prec) break;

    bool const r_assoc = is_right_assoc(op);

    ++index; // consume operator

    int const next_min_prec = r_assoc ? prec : prec + 1;

    {
      auto const err = parse_expr(out, tokens, index, next_min_prec);
      if (err) return err;
    }

    {
      auto const err = emit_binary_op(out, op);
      if (err) return err;
    }
  }

  return {};
}

auto parse_primary(vector<Op_Code>& out, vector<Token> const& tokens, size_t& index) -> Parse_Error
{
  if (index >= tokens.size()) {
    return { "unexpected end of input" };
  }

  auto const& tok = tokens[index];

  if (tok.kind == Tok_Kind::Number) {
    auto const opt = sv_to_double(tok.text);
    if (opt)
      out.emplace_back(Op_Code { .kind = Op_Kind::Val, .value = opt.value() });
    else
      Warning {}("Malformed Number");

    ++index;
    return {};
  }

  if (tok.kind == Tok_Kind::Word) {
    // takes thse out, doesnt make sense to use stack ops i think?
    // auto const it = intrinsics.find(tok.text);
    // if (it != intrinsics.end())
    //    out.push_back(it->second);
    // else
    out.emplace_back(Op_Code { .kind = Op_Kind::Word, .text = string(tok.text) });

    ++index;
    return {};
  }

  if (tok.kind == Tok_Kind::LParen) {
    ++index; // consume '('

    auto const err = parse_expr(out, tokens, index, 0);
    if (err) return err;

    if (index >= tokens.size() || tokens[index].kind != Tok_Kind::RParen) {
      return { "expected ')'" };
    }

    ++index; // consume ')'
    return {};
  }

  return { "expected expression" };
}

void parse_infix(vector<Op_Code>& out, vector<Token> const& tokens, size_t& index)
{
  vector<Op_Code> ops {};

  if (index >= tokens.size() || tokens[index].kind != Tok_Kind::LParen) {
    Warning {}("parse_infix: expected '('");
    return;
  }

  ++index; // consume '('

  auto const err = parse_expr(ops, tokens, index, 0);
  if (err) {
    Warning {}("infix parse error: ", err.text);
    return;
  }

  if (index >= tokens.size() || tokens[index].kind != Tok_Kind::RParen) {
    Warning {}("infix parse error: expected ')'");
    return;
  }

  ++index; // consume ')'

  out.insert(out.end(), ops.begin(), ops.end());
}
