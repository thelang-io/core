/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "Codegen.hpp"
#include "Error.hpp"

void codegenStmt (Codegen *codegen, const Stmt *stmt, std::size_t indent);

Codegen::~Codegen () {
  delete this->varMap;
}

void codegenBlock (Codegen *codegen, const Block *block, std::size_t indent) {
  for (const auto &stmt : block->body) {
    codegenStmt(codegen, stmt, indent);
  }
}

VarMapItemType codegenStmtExprType (const Codegen *codegen, const StmtExpr *stmtExpr) {
  if (stmtExpr->type == STMT_EXPR_EXPR) {
    auto expr = std::get<Expr *>(stmtExpr->body);

    if (expr->type == EXPR_ASSIGN) {
      auto exprAssign = std::get<ExprAssign *>(expr->body);
      return codegen->varMap->get(exprAssign->left->val).type;
    } else if (expr->type == EXPR_BINARY) {
      auto exprBinary = std::get<ExprBinary *>(expr->body);
      auto exprBinaryLeftType = codegenStmtExprType(codegen, exprBinary->left);
      auto exprBinaryRightType = codegenStmtExprType(codegen, exprBinary->right);

      if (
        exprBinaryLeftType == VAR_CHAR ||
        exprBinaryLeftType == VAR_STR ||
        exprBinaryRightType == VAR_CHAR ||
        exprBinaryRightType == VAR_STR
      ) {
        if (exprBinary->op->type == TK_OP_PLUS) {
          return VAR_STR;
        } else {
          throw Error("Tried operation other than concatenation on string");
        }
      } else if (
        exprBinaryLeftType == VAR_FLOAT ||
        exprBinaryRightType == VAR_FLOAT ||
        exprBinary->op->type == TK_OP_SLASH ||
        exprBinary->op->type == TK_OP_STAR ||
        exprBinary->op->type == TK_OP_STAR_STAR
      ) {
        return VAR_FLOAT;
      } else {
        return VAR_INT;
      }
    } else if (expr->type == EXPR_COND) {
      auto exprCond = std::get<ExprCond *>(expr->body);
      auto exprCondBodyType = codegenStmtExprType(codegen, exprCond->body);
      auto exprCondAltType = codegenStmtExprType(codegen, exprCond->alt);

      if (exprCondBodyType != exprCondAltType) {
        throw Error("Incompatible operand types");
      }

      return exprCondBodyType;
    } else if (expr->type == EXPR_UNARY) {
      auto exprUnary = std::get<ExprUnary *>(expr->body);
      auto exprType = codegenStmtExprType(codegen, exprUnary->arg);

      if (exprType != VAR_FLOAT && exprType != VAR_INT) {
        return VAR_INT;
      }

      return exprType;
    }
  } else if (stmtExpr->type == STMT_EXPR_IDENTIFIER) {
    auto id = std::get<Identifier *>(stmtExpr->body);
    return codegen->varMap->get(id->name->val).type;
  } else if (stmtExpr->type == STMT_EXPR_LITERAL) {
    auto lit = std::get<Literal *>(stmtExpr->body);

    if (lit->val->type == TK_LIT_CHAR) {
      return VAR_CHAR;
    } else if (lit->val->type == TK_LIT_FLOAT) {
      return VAR_FLOAT;
    } else if (lit->val->type == TK_LIT_INT_DEC) {
      return VAR_INT;
    } else if (lit->val->type == TK_LIT_STR) {
      return VAR_STR;
    }
  }

  throw Error("Tried to access unknown expr type");
}

void codegenStmtExpr (Codegen *codegen, const StmtExpr *stmtExpr) {
  if (stmtExpr->parenthesized) {
    codegen->mainBody += "(";
  }

  if (stmtExpr->type == STMT_EXPR_EXPR) {
    auto expr = std::get<Expr *>(stmtExpr->body);

    if (expr->type == EXPR_ASSIGN) {
      auto exprAssign = std::get<ExprAssign *>(expr->body);
      auto varName = exprAssign->left->val;

      if (exprAssign->op->type == TK_OP_STAR_STAR_EQ) {
        codegen->mainBody += varName + " = pow(" + varName + ", ";
        codegenStmtExpr(codegen, exprAssign->right);
        codegen->mainBody += ")";
      } else if (exprAssign->op->type == TK_OP_AND_AND_EQ) {
        codegen->mainBody += varName + " = " + varName + " && ";
        codegenStmtExpr(codegen, exprAssign->right);
      } else if (exprAssign->op->type == TK_OP_OR_OR_EQ) {
        codegen->mainBody += varName + " = " + varName + " || ";
        codegenStmtExpr(codegen, exprAssign->right);
      } else {
        codegen->mainBody += varName + " " + exprAssign->op->val + " ";
        codegenStmtExpr(codegen, exprAssign->right);
      }
    } else if (expr->type == EXPR_BINARY) {
      auto exprBinary = std::get<ExprBinary *>(expr->body);

      if (exprBinary->op->type == TK_OP_STAR_STAR) {
        codegen->headers.math = true;
        codegen->mainBody += "pow(";
        codegenStmtExpr(codegen, exprBinary->left);
        codegen->mainBody += ", ";
        codegenStmtExpr(codegen, exprBinary->right);
        codegen->mainBody += ")";
      } else if (exprBinary->op->type == TK_OP_SLASH || exprBinary->op->type == TK_OP_STAR) {
        codegen->mainBody += "(double) ";
        codegenStmtExpr(codegen, exprBinary->left);
        codegen->mainBody += " " + exprBinary->op->val + " (double) ";
        codegenStmtExpr(codegen, exprBinary->right);
      } else {
        codegenStmtExpr(codegen, exprBinary->left);
        codegen->mainBody += " " + exprBinary->op->val + " ";
        codegenStmtExpr(codegen, exprBinary->right);
      }
    } else if (expr->type == EXPR_CALL) {
      auto exprCall = std::get<ExprCall *>(expr->body);

      if (exprCall->callee->val == "print") {
        codegen->headers.stdio = true;
        codegen->mainBody += R"(printf(")";

        auto argIdx = 0;

        for (const auto &arg : exprCall->args) {
          codegen->mainBody += (argIdx != 0 ? "%s" : "");
          auto exprType = codegenStmtExprType(codegen, arg);

          switch (exprType) {
            case VAR_CHAR: {
              codegen->mainBody += "%c";
              break;
            }
            case VAR_FLOAT: {
              codegen->mainBody += "%f";
              break;
            }
            case VAR_INT: {
              codegen->mainBody += "%ld";
              break;
            }
            default: {
              codegen->mainBody += "%s";
              break;
            }
          }

          argIdx++;
        }

        codegen->mainBody += R"(%s", )";
        argIdx = 0;

        for (const auto &arg : exprCall->args) {
          codegen->mainBody += (argIdx != 0 ? R"(" ", )" : "");
          codegenStmtExpr(codegen, arg);
          codegen->mainBody += ", ";

          argIdx++;
        }

        codegen->mainBody += R"("\n"))";
      } else {
        throw Error("Tried to access unknown built-in function");
      }
    } else if (expr->type == EXPR_COND) {
      auto exprCond = std::get<ExprCond *>(expr->body);

      codegenStmtExpr(codegen, exprCond->cond);
      codegen->mainBody += " ? ";
      codegenStmtExpr(codegen, exprCond->body);
      codegen->mainBody += " : ";
      codegenStmtExpr(codegen, exprCond->alt);
    } else if (expr->type == EXPR_UNARY) {
      auto exprUnary = std::get<ExprUnary *>(expr->body);

      if (exprUnary->prefix) {
        codegen->mainBody += exprUnary->op->val;
        codegenStmtExpr(codegen, exprUnary->arg);
      } else {
        codegenStmtExpr(codegen, exprUnary->arg);
        codegen->mainBody += exprUnary->op->val;
      }
    }
  } else if (stmtExpr->type == STMT_EXPR_IDENTIFIER) {
    auto id = std::get<Identifier *>(stmtExpr->body);
    codegen->mainBody += id->name->val;
  } else if (stmtExpr->type == STMT_EXPR_LITERAL) {
    auto lit = std::get<Literal *>(stmtExpr->body);
    codegen->mainBody += lit->val->val;
  } else {
    throw Error("Tried to access unknown expr");
  }

  if (stmtExpr->parenthesized) {
    codegen->mainBody += ")";
  }
}

void codegenStmtIf (Codegen *codegen, const StmtIf *stmtIf, std::size_t indent) {
  codegen->mainBody += "if (";
  codegenStmtExpr(codegen, stmtIf->cond);
  codegen->mainBody += ") {\n";
  codegenBlock(codegen, stmtIf->body, indent + 2);

  if (stmtIf->alt != nullptr) {
    codegen->mainBody += std::string(indent, ' ') + "} else ";

    if (stmtIf->alt->type == COND_BLOCK) {
      codegen->mainBody += "{\n";
      codegenBlock(codegen, std::get<Block *>(stmtIf->alt->body), indent + 2);
      codegen->mainBody += std::string(indent, ' ') + "}";
    } else if (stmtIf->alt->type == COND_STMT_IF) {
      codegenStmtIf(codegen, std::get<StmtIf *>(stmtIf->alt->body), indent);
    }
  } else {
    codegen->mainBody += std::string(indent, ' ') + "}";
  }
}

void codegenStmt (Codegen *codegen, const Stmt *stmt, std::size_t indent) {
  codegen->mainBody += std::string(indent, ' ');

  if (stmt->type == STMT_EXPR) {
    codegenStmtExpr(codegen, std::get<StmtExpr *>(stmt->body));
    codegen->mainBody += ";\n";
  } else if (stmt->type == STMT_IF) {
    codegenStmtIf(codegen, std::get<StmtIf *>(stmt->body), indent);
    codegen->mainBody += "\n";
  } else if (stmt->type == STMT_RETURN) {
    auto stmtReturn = std::get<StmtReturn *>(stmt->body);

    codegen->mainBody += "return ";
    codegenStmtExpr(codegen, stmtReturn->arg);
    codegen->mainBody += ";\n";
  } else if (stmt->type == STMT_SHORT_VAR_DECL) {
    auto stmtShortVarDecl = std::get<StmtShortVarDecl *>(stmt->body);
    auto varName = stmtShortVarDecl->id->val;
    auto exprType = codegenStmtExprType(codegen, stmtShortVarDecl->init);

    switch (exprType) {
      case VAR_CHAR: {
        codegen->mainBody += stmtShortVarDecl->mut ? "char " : "const char ";
        break;
      }
      case VAR_FLOAT: {
        codegen->mainBody += stmtShortVarDecl->mut ? "double " : "const double ";
        break;
      }
      case VAR_INT: {
        codegen->mainBody += stmtShortVarDecl->mut ? "long " : "const long ";
        break;
      }
      default: {
        codegen->mainBody += stmtShortVarDecl->mut ? "char *" : "const char *";
        break;
      }
    }

    codegen->mainBody += varName + " = ";
    codegenStmtExpr(codegen, stmtShortVarDecl->init);
    codegen->mainBody += ";\n";
    codegen->varMap->add(exprType, varName);
  } else {
    throw Error("Tried to access unknown stmt type");
  }
}

std::string codegen (const AST *ast) {
  auto codegen = new Codegen{{}, "", new VarMap{}};

  if (ast->mainPresent) {
    codegenBlock(codegen, ast->mainBody, 2);
  }

  auto code = std::string(codegen->headers.math ? "#include <math.h>\n" : "") +
    std::string(codegen->headers.stdio ? "#include <stdio.h>\n" : "") +
    "\nint main (const int argc, const char **argv) {\n" + codegen->mainBody + "}\n";

  delete codegen;
  return code;
}
