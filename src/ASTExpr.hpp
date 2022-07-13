/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef SRC_AST_EXPR_HPP
#define SRC_AST_EXPR_HPP

#include "VarMap.hpp"

enum ASTExprAssignOp {
  AST_EXPR_ASSIGN_ADD,
  AST_EXPR_ASSIGN_AND,
  AST_EXPR_ASSIGN_BIT_AND,
  AST_EXPR_ASSIGN_BIT_OR,
  AST_EXPR_ASSIGN_BIT_XOR,
  AST_EXPR_ASSIGN_DIV,
  AST_EXPR_ASSIGN_EQ,
  AST_EXPR_ASSIGN_LSHIFT,
  AST_EXPR_ASSIGN_MOD,
  AST_EXPR_ASSIGN_MUL,
  AST_EXPR_ASSIGN_OR,
  AST_EXPR_ASSIGN_RSHIFT,
  AST_EXPR_ASSIGN_SUB
};

enum ASTExprBinaryOp {
  AST_EXPR_BINARY_ADD,
  AST_EXPR_BINARY_AND,
  AST_EXPR_BINARY_BIT_AND,
  AST_EXPR_BINARY_BIT_OR,
  AST_EXPR_BINARY_BIT_XOR,
  AST_EXPR_BINARY_DIV,
  AST_EXPR_BINARY_EQ,
  AST_EXPR_BINARY_GE,
  AST_EXPR_BINARY_GT,
  AST_EXPR_BINARY_LSHIFT,
  AST_EXPR_BINARY_LE,
  AST_EXPR_BINARY_LT,
  AST_EXPR_BINARY_MOD,
  AST_EXPR_BINARY_MUL,
  AST_EXPR_BINARY_NE,
  AST_EXPR_BINARY_OR,
  AST_EXPR_BINARY_RSHIFT,
  AST_EXPR_BINARY_SUB
};

enum ASTExprLitType {
  AST_EXPR_LIT_BOOL,
  AST_EXPR_LIT_CHAR,
  AST_EXPR_LIT_FLOAT,
  AST_EXPR_LIT_INT_BIN,
  AST_EXPR_LIT_INT_DEC,
  AST_EXPR_LIT_INT_HEX,
  AST_EXPR_LIT_INT_OCT,
  AST_EXPR_LIT_STR
};

enum ASTExprUnaryOp {
  AST_EXPR_UNARY_BIT_NOT,
  AST_EXPR_UNARY_DECREMENT,
  AST_EXPR_UNARY_INCREMENT,
  AST_EXPR_UNARY_MINUS,
  AST_EXPR_UNARY_NOT,
  AST_EXPR_UNARY_PLUS
};

struct ASTExprAccess;
struct ASTExprAssign;
struct ASTExprBinary;
struct ASTExprCall;
struct ASTExprCond;
struct ASTExprLit;
struct ASTExprObj;
struct ASTExprUnary;
struct ASTMember;

using ASTExpr = std::variant<
  ASTExprAccess,
  ASTExprAssign,
  ASTExprBinary,
  ASTExprCall,
  ASTExprCond,
  ASTExprLit,
  ASTExprObj,
  ASTExprUnary
>;

struct ASTNodeExpr {
  Type *type;
  std::shared_ptr<ASTExpr> body;
  bool parenthesized = false;

  bool isLit () const;
  std::string litBody () const;
  std::string xml (std::size_t = 0) const;
};

using ASTMemberObj = std::variant<std::shared_ptr<Var>, ASTMember>;

struct ASTMember {
  std::shared_ptr<ASTMemberObj> obj;
  std::string prop;
};

struct ASTExprAccess {
  std::shared_ptr<ASTMemberObj> body;

  std::string xml (std::size_t = 0) const;
};

struct ASTExprAssign {
  ASTExprAccess left;
  ASTExprAssignOp op;
  ASTNodeExpr right;
};

struct ASTExprBinary {
  ASTNodeExpr left;
  ASTExprBinaryOp op;
  ASTNodeExpr right;
};

struct ASTExprCallArg {
  std::optional<std::string> id;
  ASTNodeExpr expr;
};

struct ASTExprCall {
  ASTExprAccess callee;
  Type *calleeType;
  std::vector<ASTExprCallArg> args;
};

struct ASTExprCond {
  ASTNodeExpr cond;
  ASTNodeExpr body;
  ASTNodeExpr alt;
};

struct ASTExprLit {
  ASTExprLitType type;
  std::string body;
};

struct ASTExprObjProp {
  std::string id;
  ASTNodeExpr init;
};

struct ASTExprObj {
  Type *type;
  std::vector<ASTExprObjProp> props;
};

struct ASTExprUnary {
  ASTNodeExpr arg;
  ASTExprUnaryOp op;
  bool prefix = false;
};

#endif
