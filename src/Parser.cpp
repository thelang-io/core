/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "Parser.hpp"

Parser::Parser (Lexer *l) {
  this->lexer = l;
  this->reader = this->lexer->reader;
}

ParserStmt Parser::next () {
  auto [_0, tok0] = this->lexer->next();

  if (tok0.type == TK_EOF) {
    return this->_stmt(ParserStmtEof{}, tok0.start);
  } else if (tok0.type == TK_KW_BREAK) {
    return this->_stmt(ParserStmtBreak{}, tok0.start);
  } else if (tok0.type == TK_KW_CONTINUE) {
    return this->_stmt(ParserStmtContinue{}, tok0.start);
  } else if (tok0.type == TK_KW_IF) {
    auto stmtIf = this->_stmtIf();
    return this->_stmt(stmtIf, tok0.start);
  } else if (tok0.type == TK_KW_MAIN) {
    auto mainBody = this->_block();
    return this->_stmt(ParserStmtMain{mainBody}, tok0.start);
  } else if (tok0.type == TK_KW_RETURN) {
    auto loc1 = this->lexer->loc;
    auto returnBody = std::optional<ParserStmtExpr>{};

    try {
      returnBody = this->_stmtExpr();
    } catch (const Error &err) {
      this->lexer->seek(loc1);
    }

    return this->_stmt(ParserStmtReturn{returnBody}, tok0.start);
  }

  if (tok0.type == TK_KW_FN) {
    auto [_1, tok1] = this->lexer->next();

    if (tok1.type != TK_ID) {
      throw Error(this->reader, tok1.start, E0115);
    }

    auto [_2, tok2] = this->lexer->next();

    if (tok2.type != TK_OP_LPAR) {
      throw Error(this->reader, tok2.start, E0116);
    }

    auto [_3, tok3] = this->lexer->next();
    auto fnDeclParams = std::vector<ParserStmtFnDeclParam>();

    while (tok3.type != TK_OP_RPAR) {
      if (tok3.type != TK_ID) {
        throw Error(this->reader, tok3.start, E0117);
      }

      auto fnDeclParamType = std::optional<std::shared_ptr<ParserType>>{};
      auto fnDeclParamVariadic = false;
      auto fnDeclParamInit = std::optional<ParserStmtExpr>{};
      auto [_4, tok4] = this->lexer->next();

      if (tok4.type == TK_OP_COLON) {
        auto typeTest1 = this->_type();

        if (typeTest1 == nullptr) {
          throw Error(this->reader, this->lexer->loc, E0118);
        }

        fnDeclParamType = typeTest1;
        auto [loc5, tok5] = this->lexer->next();

        if (tok5.type == TK_OP_EQ) {
          fnDeclParamInit = this->_stmtExpr();
        } else if (tok5.type == TK_OP_DOT_DOT_DOT) {
          fnDeclParamVariadic = true;

          auto [loc6, tok6] = this->lexer->next();

          if (tok6.type == TK_OP_EQ) {
            fnDeclParamInit = this->_stmtExpr();
          } else {
            this->lexer->seek(loc6);
          }
        } else {
          this->lexer->seek(loc5);
        }
      } else if (tok4.type == TK_OP_COLON_EQ) {
        fnDeclParamInit = this->_stmtExpr();
      } else {
        throw Error(this->reader, tok4.start, E0119);
      }

      fnDeclParams.push_back(ParserStmtFnDeclParam{tok3, fnDeclParamType, fnDeclParamVariadic, fnDeclParamInit});
      std::tie(_3, tok3) = this->lexer->next();

      if (tok3.type == TK_OP_COMMA) {
        std::tie(_3, tok3) = this->lexer->next();
      }
    }

    auto typeTest2 = this->_type();
    auto fnDeclReturnType = typeTest2 == nullptr ? std::optional<std::shared_ptr<ParserType>>{} : typeTest2;
    auto fnDeclBody = this->_block();

    return this->_stmt(ParserStmtFnDecl{tok1, fnDeclParams, fnDeclReturnType, fnDeclBody}, tok0.start);
  }

  if (tok0.type == TK_KW_LOOP) {
    auto [loc1, tok1] = this->lexer->next();

    if (tok1.type == TK_OP_LBRACE) {
      this->lexer->seek(loc1);
      auto loopBody = this->_block();

      return this->_stmt(ParserStmtLoop{std::nullopt, std::nullopt, std::nullopt, loopBody}, tok0.start);
    } else if (tok1.type == TK_OP_SEMI) {
      auto stmtLoop = this->_stmtLoop(std::nullopt);
      return this->_stmt(stmtLoop, tok0.start);
    }

    this->lexer->seek(loc1);
    auto loopInit = this->next();

    if (!std::holds_alternative<ParserStmtExpr>(loopInit.body) && !std::holds_alternative<ParserStmtVarDecl>(loopInit.body)) {
      throw Error(this->reader, tok1.start, E0105);
    }

    auto [loc2, tok2] = this->lexer->next();

    if (std::holds_alternative<ParserStmtVarDecl>(loopInit.body) && tok2.type != TK_OP_SEMI) {
      throw Error(this->reader, tok2.start, E0106);
    } else if (std::holds_alternative<ParserStmtExpr>(loopInit.body) && tok2.type != TK_OP_LBRACE && tok2.type != TK_OP_SEMI) {
      throw Error(this->reader, tok2.start, E0107);
    } else if (std::holds_alternative<ParserStmtExpr>(loopInit.body) && tok2.type == TK_OP_LBRACE) {
      this->lexer->seek(loc2);

      auto loopCond = std::get<ParserStmtExpr>(loopInit.body);
      auto loopBody = this->_block();

      return this->_stmt(ParserStmtLoop{std::nullopt, loopCond, std::nullopt, loopBody}, tok0.start);
    }

    auto stmtLoop = this->_stmtLoop(std::make_shared<ParserStmt>(loopInit));
    return this->_stmt(stmtLoop, tok0.start);
  }

  if (tok0.type == TK_KW_MUT) {
    auto [loc1, tok1] = this->lexer->next();

    if (tok1.type == TK_ID) {
      auto [_2, tok2] = this->lexer->next();

      if (tok2.type == TK_OP_COLON) {
        auto varDeclType = this->_type();

        if (varDeclType == nullptr) {
          throw Error(this->reader, this->lexer->loc, E0102);
        }

        auto [loc3, tok3] = this->lexer->next();
        auto varDeclInit = std::optional<ParserStmtExpr>{};

        if (tok3.type == TK_OP_EQ) {
          varDeclInit = this->_stmtExpr();
        } else {
          this->lexer->seek(loc3);
        }

        return this->_stmt(ParserStmtVarDecl{tok1, varDeclType, varDeclInit, true}, tok0.start);
      } else if (tok2.type == TK_OP_COLON_EQ) {
        auto varDeclInit = this->_stmtExpr();
        return this->_stmt(ParserStmtVarDecl{tok1, std::nullopt, varDeclInit, true}, tok0.start);
      }
    }

    this->lexer->seek(loc1);
  }

  if (tok0.type == TK_KW_OBJ) {
    auto [_1, tok1] = this->lexer->next();

    if (tok1.type != TK_ID) {
      throw Error(this->reader, tok1.start, E0121);
    }

    auto [_2, tok2] = this->lexer->next();

    if (tok2.type != TK_OP_LBRACE) {
      throw Error(this->reader, tok2.start, E0122);
    }

    auto [_3, tok3] = this->lexer->next();
    auto objDeclFields = std::vector<ParserStmtObjDeclField>();

    while (tok3.type != TK_OP_RBRACE) {
      if (tok3.type != TK_ID) {
        throw Error(this->reader, tok3.start, E0123);
      }

      auto [_4, tok4] = this->lexer->next();

      if (tok4.type != TK_OP_COLON) {
        throw Error(this->reader, tok4.start, E0124);
      }

      auto objDeclFieldType = this->_type();

      if (objDeclFieldType == nullptr) {
        throw Error(this->reader, this->lexer->loc, E0125);
      }

      objDeclFields.push_back(ParserStmtObjDeclField{tok3, objDeclFieldType});
      std::tie(_3, tok3) = this->lexer->next();
    }

    if (objDeclFields.empty()) {
      throw Error(this->reader, tok0.start, E0126);
    }

    return this->_stmt(ParserStmtObjDecl{tok1, objDeclFields}, tok0.start);
  }

  if (tok0.type == TK_ID) {
    auto [loc1, tok1] = this->lexer->next();

    if (tok1.type == TK_OP_COLON) {
      auto varDeclType = this->_type();

      if (varDeclType == nullptr) {
        throw Error(this->reader, this->lexer->loc, E0102);
      }

      auto [loc2, tok2] = this->lexer->next();
      auto varDeclInit = std::optional<ParserStmtExpr>{};

      if (tok2.type == TK_OP_EQ) {
        varDeclInit = this->_stmtExpr();
      } else {
        this->lexer->seek(loc2);
      }

      return this->_stmt(ParserStmtVarDecl{tok0, varDeclType, varDeclInit}, tok0.start);
    } else if (tok1.type == TK_OP_COLON_EQ) {
      auto varDeclInit = this->_stmtExpr();
      return this->_stmt(ParserStmtVarDecl{tok0, std::nullopt, varDeclInit}, tok0.start);
    }

    this->lexer->seek(loc1);
  }

  this->lexer->seek(tok0.start);

  try {
    auto stmtExpr = this->_stmtExpr();
    return this->_stmt(stmtExpr, tok0.start);
  } catch (const Error &err) {
    this->lexer->seek(tok0.start);
  }

  throw Error(this->reader, tok0.start, E0100);
}

ParserBlock Parser::_block () {
  auto [_1, tok1] = this->lexer->next();

  if (tok1.type != TK_OP_LBRACE) {
    throw Error(this->reader, tok1.start, E0103);
  }

  auto block = ParserBlock{};

  while (true) {
    auto [loc2, tok2] = this->lexer->next();

    if (tok2.type == TK_EOF) {
      throw Error(this->reader, tok2.start, E0104);
    } else if (tok2.type == TK_OP_RBRACE) {
      break;
    }

    this->lexer->seek(loc2);

    auto stmt = this->next();
    block.push_back(stmt);
  }

  return block;
}

ParserStmt Parser::_stmt (const ParserStmtBody &body, ReaderLocation start) const {
  return ParserStmt{body, start, this->lexer->loc};
}

ParserStmtExpr Parser::_stmtExpr (bool singleStmt) {
  auto [_1, tok1] = this->lexer->next();

  if (
    tok1.type == TK_KW_FALSE ||
    tok1.type == TK_KW_TRUE ||
    tok1.type == TK_LIT_CHAR ||
    tok1.type == TK_LIT_FLOAT ||
    tok1.type == TK_LIT_INT_BIN ||
    tok1.type == TK_LIT_INT_DEC ||
    tok1.type == TK_LIT_INT_HEX ||
    tok1.type == TK_LIT_INT_OCT ||
    tok1.type == TK_LIT_STR
  ) {
    auto exprLit = std::make_shared<ParserExpr>(ParserExprLit{tok1});
    auto stmtExpr = ParserStmtExpr{exprLit};

    return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
  }

  if (
    tok1.type == TK_OP_EXCL ||
    tok1.type == TK_OP_EXCL_EXCL ||
    tok1.type == TK_OP_MINUS ||
    tok1.type == TK_OP_MINUS_MINUS ||
    tok1.type == TK_OP_PLUS ||
    tok1.type == TK_OP_PLUS_PLUS ||
    tok1.type == TK_OP_TILDE
  ) {
    auto exprUnaryArg = this->_stmtExpr(true);
    auto exprUnary = std::make_shared<ParserExpr>(ParserExprUnary{exprUnaryArg, tok1, true});
    auto stmtExpr = ParserStmtExpr{exprUnary};

    return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
  }

  if (tok1.type == TK_ID) {
    auto exprAccess = ParserExprAccess{tok1};
    auto [loc2, tok2] = this->lexer->next();

    while (tok2.type == TK_OP_DOT) {
      auto [_3, tok3] = this->lexer->next();

      if (tok3.type != TK_ID) {
        throw Error(this->reader, tok3.start, E0110);
      }

      auto member = ParserMember{std::make_shared<ParserMemberObj>(exprAccess.body), tok3};
      exprAccess = ParserExprAccess{member};
      std::tie(loc2, tok2) = this->lexer->next();
    }

    this->lexer->seek(loc2);
    auto [loc4, tok4] = this->lexer->next();

    if (
      tok4.type == TK_OP_AND_AND_EQ ||
      tok4.type == TK_OP_AND_EQ ||
      tok4.type == TK_OP_CARET_EQ ||
      tok4.type == TK_OP_EQ ||
      tok4.type == TK_OP_LSHIFT_EQ ||
      tok4.type == TK_OP_MINUS_EQ ||
      tok4.type == TK_OP_OR_EQ ||
      tok4.type == TK_OP_OR_OR_EQ ||
      tok4.type == TK_OP_PERCENT_EQ ||
      tok4.type == TK_OP_PLUS_EQ ||
      tok4.type == TK_OP_QN_QN_EQ ||
      tok4.type == TK_OP_RSHIFT_EQ ||
      tok4.type == TK_OP_SLASH_EQ ||
      tok4.type == TK_OP_STAR_EQ ||
      tok4.type == TK_OP_STAR_STAR_EQ
    ) {
      auto exprAssignRight = this->_stmtExpr();
      auto exprAssign = std::make_shared<ParserExpr>(ParserExprAssign{exprAccess, tok4, exprAssignRight});
      auto stmtExpr = ParserStmtExpr{exprAssign};

      return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
    }

    if (tok4.type == TK_OP_LBRACE) {
      auto [_5, tok5] = this->lexer->next();
      auto exprObjProps = std::vector<ParserExprObjProp>();

      while (tok5.type != TK_OP_RBRACE) {
        if (tok5.type != TK_ID && exprObjProps.empty()) {
          this->lexer->seek(loc4);

          auto expr = std::make_shared<ParserExpr>(exprAccess);
          auto stmtExpr = ParserStmtExpr{expr};

          return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
        } else if (tok5.type != TK_ID) {
          throw Error(this->reader, tok5.start, E0112);
        }

        auto [_6, tok6] = this->lexer->next();

        if (tok6.type != TK_OP_COLON && exprObjProps.empty()) {
          this->lexer->seek(loc4);

          auto expr = std::make_shared<ParserExpr>(exprAccess);
          auto stmtExpr = ParserStmtExpr{expr};

          return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
        } else if (tok6.type != TK_OP_COLON) {
          throw Error(this->reader, tok6.start, E0113);
        }

        auto exprObjPropInit = this->_stmtExpr();
        exprObjProps.push_back(ParserExprObjProp{tok5, exprObjPropInit});

        std::tie(_5, tok5) = this->lexer->next();

        if (tok5.type == TK_OP_COMMA) {
          std::tie(_5, tok5) = this->lexer->next();
        }
      }

      if (!std::holds_alternative<Token>(exprAccess.body)) {
        throw Error(this->reader, tok1.start, E0114);
      }

      auto exprObj = std::make_shared<ParserExpr>(ParserExprObj{std::get<Token>(exprAccess.body), exprObjProps});
      auto stmtExpr = ParserStmtExpr{exprObj};

      return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
    }

    if (tok4.type == TK_OP_LPAR) {
      auto [loc5, tok5] = this->lexer->next();
      auto exprCallArgs = std::vector<ParserExprCallArg>();

      while (tok5.type != TK_OP_RPAR) {
        this->lexer->seek(loc5);

        auto exprCallArgId = std::optional<Token>{};
        auto [loc6, tok6] = this->lexer->next();

        if (tok6.type == TK_ID) {
          auto [_7, tok7] = this->lexer->next();

          if (tok7.type == TK_OP_COLON) {
            exprCallArgId = tok6;
          } else {
            this->lexer->seek(loc6);
          }
        } else {
          this->lexer->seek(loc6);
        }

        auto exprCallArgExpr = this->_stmtExpr();
        exprCallArgs.push_back(ParserExprCallArg{exprCallArgId, exprCallArgExpr});

        std::tie(loc5, tok5) = this->lexer->next();

        if (tok5.type == TK_OP_COMMA) {
          std::tie(loc5, tok5) = this->lexer->next();
        }
      }

      auto exprCall = std::make_shared<ParserExpr>(ParserExprCall{exprAccess, exprCallArgs});
      auto stmtExpr = ParserStmtExpr{exprCall};

      return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
    }

    if (tok4.type == TK_OP_MINUS_MINUS || tok4.type == TK_OP_PLUS_PLUS) {
      auto exprUnaryArgExpr = std::make_shared<ParserExpr>(exprAccess);
      auto exprUnaryArg = ParserStmtExpr{exprUnaryArgExpr};
      auto exprUnary = std::make_shared<ParserExpr>(ParserExprUnary{exprUnaryArg, tok4});
      auto stmtExpr = ParserStmtExpr{exprUnary};

      return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
    }

    this->lexer->seek(loc4);

    auto expr = std::make_shared<ParserExpr>(exprAccess);
    auto stmtExpr = ParserStmtExpr{expr};

    return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
  }

  if (tok1.type == TK_OP_LPAR) {
    auto stmtExpr = this->_stmtExpr();
    auto [_2, tok2] = this->lexer->next();

    if (tok2.type != TK_OP_RPAR) {
      throw Error(this->reader, tok2.start, E0109);
    }

    stmtExpr.parenthesized = true;
    return singleStmt ? stmtExpr : this->_wrapStmtExpr(stmtExpr);
  }

  throw Error(this->reader, tok1.start, E0101);
}

ParserStmtIf Parser::_stmtIf () {
  auto cond = std::make_shared<ParserStmt>(this->next());
  auto body = this->_block();
  auto alt = std::optional<std::shared_ptr<ParserStmtIfCond>>{};
  auto [loc1, tok1] = this->lexer->next();

  if (tok1.type == TK_KW_ELIF) {
    auto stmtElifCond = this->_stmtIf();
    alt = std::make_shared<ParserStmtIfCond>(stmtElifCond);
  } else if (tok1.type == TK_KW_ELSE) {
    auto stmtElseCond = this->_block();
    alt = std::make_shared<ParserStmtIfCond>(stmtElseCond);
  } else {
    this->lexer->seek(loc1);
  }

  return ParserStmtIf{cond, body, alt};
}

ParserStmtLoop Parser::_stmtLoop (const std::optional<std::shared_ptr<ParserStmt>> &init) {
  auto cond = std::optional<ParserStmtExpr>{};
  auto upd = std::optional<ParserStmtExpr>{};
  auto [loc1, tok1] = this->lexer->next();

  if (tok1.type != TK_OP_SEMI) {
    this->lexer->seek(loc1);

    cond = this->_stmtExpr();
    auto [_2, tok2] = this->lexer->next();

    if (tok2.type != TK_OP_SEMI) {
      throw Error(this->reader, tok2.start, E0108);
    }
  }

  auto [loc3, tok3] = this->lexer->next();
  this->lexer->seek(loc3);

  if (tok3.type != TK_OP_LBRACE) {
    upd = this->_stmtExpr();
  }

  auto body = this->_block();
  return ParserStmtLoop{init, cond, upd, body};
}

std::shared_ptr<ParserType> Parser::_type () {
  auto [loc1, tok1] = this->lexer->next();
  auto typeId = std::optional<ParserTypeId>{};

  if (tok1.type == TK_OP_LPAR) {
    auto type = this->_type();

    if (type == nullptr) {
      this->lexer->seek(loc1);
      return nullptr;
    }

    auto [loc2, tok2] = this->lexer->next();

    if (tok2.type != TK_OP_RPAR) {
      this->lexer->seek(loc2);
      throw Error(this->reader, this->lexer->loc, E0127);
    }

    return std::make_shared<ParserType>(ParserType{type->body, true, tok1.start, this->lexer->loc});
  } else if (tok1.type == TK_KW_FN) {
    auto [loc2, tok2] = this->lexer->next();

    if (tok2.type == TK_OP_LPAR) {
      auto [loc3, tok3] = this->lexer->next();
      auto fnParams = std::vector<ParserTypeFnParam>{};

      while (tok3.type != TK_OP_RPAR) {
        this->lexer->seek(loc3);
        auto fnParamType = this->_type();
        auto fnParamVariadic = false;

        if (fnParamType == nullptr) {
          throw Error(this->reader, this->lexer->loc, E0118);
        }

        auto [loc4, tok4] = this->lexer->next();

        if (tok4.type == TK_OP_DOT_DOT_DOT) {
          fnParamVariadic = true;
        } else {
          this->lexer->seek(loc4);
        }

        fnParams.push_back(ParserTypeFnParam{fnParamType, fnParamVariadic});
        std::tie(loc3, tok3) = this->lexer->next();

        if (tok3.type == TK_OP_COMMA) {
          std::tie(loc3, tok3) = this->lexer->next();
        }
      }

      auto fnReturnType = this->_type();

      if (fnReturnType == nullptr) {
        throw Error(this->reader, this->lexer->loc, E0120);
      }

      auto typeFn = ParserTypeFn{fnParams, fnReturnType};
      return std::make_shared<ParserType>(ParserType{typeFn, false, tok1.start, this->lexer->loc});
    } else {
      this->lexer->seek(loc2);
    }

    typeId = ParserTypeId{tok1};
  } else if (tok1.type == TK_ID) {
    typeId = ParserTypeId{tok1};
  }

  if (typeId == std::nullopt) {
    this->lexer->seek(loc1);
    return nullptr;
  }

  return std::make_shared<ParserType>(ParserType{*typeId, false, tok1.start, this->lexer->loc});
}

ParserStmtExpr Parser::_wrapStmtExpr (const ParserStmtExpr &stmtExpr) {
  auto [loc1, tok1] = this->lexer->next();

  if (
    tok1.type == TK_OP_AND ||
    tok1.type == TK_OP_AND_AND ||
    tok1.type == TK_OP_CARET ||
    tok1.type == TK_OP_EQ_EQ ||
    tok1.type == TK_OP_EXCL_EQ ||
    tok1.type == TK_OP_GT ||
    tok1.type == TK_OP_GT_EQ ||
    tok1.type == TK_OP_LSHIFT ||
    tok1.type == TK_OP_LT ||
    tok1.type == TK_OP_LT_EQ ||
    tok1.type == TK_OP_MINUS ||
    tok1.type == TK_OP_OR ||
    tok1.type == TK_OP_OR_OR ||
    tok1.type == TK_OP_PERCENT ||
    tok1.type == TK_OP_PLUS ||
    tok1.type == TK_OP_QN_QN ||
    tok1.type == TK_OP_RSHIFT ||
    tok1.type == TK_OP_SLASH ||
    tok1.type == TK_OP_STAR ||
    tok1.type == TK_OP_STAR_STAR
  ) {
    auto stmtExprRight = this->_stmtExpr(true);

    if (std::holds_alternative<ParserExprBinary>(*stmtExpr.body) && !stmtExpr.parenthesized) {
      auto exprBinaryLeft = std::get<ParserExprBinary>(*stmtExpr.body);

      if (exprBinaryLeft.op.precedence() < tok1.precedence()) {
        auto newExprBinaryRight = std::make_shared<ParserExpr>(ParserExprBinary{exprBinaryLeft.right, tok1, stmtExprRight});
        auto newStmtExprRight = ParserStmtExpr{newExprBinaryRight};
        auto exprBinary = std::make_shared<ParserExpr>(ParserExprBinary{exprBinaryLeft.left, exprBinaryLeft.op, newStmtExprRight});
        auto newStmtExpr = ParserStmtExpr{exprBinary};

        return this->_wrapStmtExpr(newStmtExpr);
      }
    }

    auto exprBinary = std::make_shared<ParserExpr>(ParserExprBinary{stmtExpr, tok1, stmtExprRight});
    auto newStmtExpr = ParserStmtExpr{exprBinary};

    return this->_wrapStmtExpr(newStmtExpr);
  }

  if (tok1.type == TK_OP_QN) {
    auto exprCondBody = this->_stmtExpr();
    auto [_2, tok2] = this->lexer->next();

    if (tok2.type != TK_OP_COLON) {
      throw Error(this->reader, tok2.start, E0111);
    }

    auto exprCondAlt = this->_stmtExpr();
    auto exprCond = std::make_shared<ParserExpr>(ParserExprCond{stmtExpr, exprCondBody, exprCondAlt});
    auto newStmtExpr = ParserStmtExpr{exprCond};

    return this->_wrapStmtExpr(newStmtExpr);
  }

  this->lexer->seek(loc1);
  return stmtExpr;
}
