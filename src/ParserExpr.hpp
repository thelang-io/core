/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef SRC_PARSER_EXPR_HPP
#define SRC_PARSER_EXPR_HPP

#include <variant>
#include <vector>
#include "Lexer.hpp"

struct ParserExprAccess;
struct ParserExprAssign;
struct ParserExprBinary;
struct ParserExprCall;
struct ParserExprCallArg;
struct ParserExprCond;
struct ParserExprLit;
struct ParserExprObj;
struct ParserExprObjProp;
struct ParserExprUnary;
struct ParserMember;
struct ParserStmtExpr;

using ParserExpr = std::variant<
  // TODO Test
  ParserExprAccess,
  // TODO Test
  ParserExprAssign,
  // TODO Test
  ParserExprBinary,
  // TODO Test
  ParserExprCall,
  // TODO Test
  ParserExprCond,
  // TODO Test
  ParserExprLit,
  // TODO Test
  ParserExprObj,
  // TODO Test
  ParserExprUnary
>;

struct ParserMember {
  std::shared_ptr<std::variant<Token, ParserMember>> obj;
  Token prop;
};

struct ParserStmtExpr {
  std::shared_ptr<ParserExpr> body;
  bool parenthesized = false;

  std::string xml (std::size_t = 0) const;
};

struct ParserExprAccess {
  std::variant<Token, ParserMember> body;
};

struct ParserExprAssign {
  ParserExprAccess left;
  Token op;
  ParserStmtExpr right;
};

struct ParserExprBinary {
  ParserStmtExpr left;
  Token op;
  ParserStmtExpr right;
};

struct ParserExprCall {
  ParserExprAccess callee;
  std::vector<ParserExprCallArg> args;
};

struct ParserExprCallArg {
  std::optional<Token> id;
  ParserStmtExpr expr;
};

struct ParserExprCond {
  ParserStmtExpr cond;
  ParserStmtExpr body;
  ParserStmtExpr alt;
};

struct ParserExprLit {
  Token body;
};

struct ParserExprObj {
  Token id;
  std::vector<ParserExprObjProp> props;
};

struct ParserExprObjProp {
  Token id;
  ParserStmtExpr init;
};

struct ParserExprUnary {
  ParserStmtExpr arg;
  Token op;
  bool prefix = false;
};

#endif
