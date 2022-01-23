/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "Error.hpp"
#include "Lexer.hpp"

Lexer::Lexer (Reader *reader) {
  this->_reader = reader;
}

Token Lexer::next () {
  this->_whitespace();
  this->loc = this->_reader->loc;
  this->val = "";

  if (this->_reader->eof()) {
    return this->_tok(TK_EOF);
  }

  auto ch = this->_reader->next();
  this->val += ch;

  if (ch == '&') return this->_opEq2('&', TK_OP_AND, TK_OP_AND_EQ, TK_OP_AND_AND, TK_OP_AND_AND_EQ);
  if (ch == '^') return this->_opEq(TK_OP_CARET, TK_OP_CARET_EQ);
  if (ch == ':') return this->_opEq(TK_OP_COLON, TK_OP_COLON_EQ);
  if (ch == ',') return this->_tok(TK_OP_COMMA);
  if (ch == '=') return this->_opEq(TK_OP_EQ, TK_OP_EQ_EQ);
  if (ch == '!') return this->_opEq2('!', TK_OP_EXCL, TK_OP_EXCL_EQ, TK_OP_EXCL_EXCL);
  if (ch == '>') return this->_opEq2('>', TK_OP_GT, TK_OP_GT_EQ, TK_OP_RSHIFT, TK_OP_RSHIFT_EQ);
  if (ch == '{') return this->_tok(TK_OP_LBRACE);
  if (ch == '[') return this->_tok(TK_OP_LBRACK);
  if (ch == '(') return this->_tok(TK_OP_LPAR);
  if (ch == '<') return this->_opEq2('<', TK_OP_LT, TK_OP_LT_EQ, TK_OP_LSHIFT, TK_OP_LSHIFT_EQ);
  if (ch == '-') return this->_opEq2('-', TK_OP_MINUS, TK_OP_MINUS_EQ, TK_OP_MINUS_MINUS);
  if (ch == '|') return this->_opEq2('|', TK_OP_OR, TK_OP_OR_EQ, TK_OP_OR_OR, TK_OP_OR_OR_EQ);
  if (ch == '%') return this->_opEq(TK_OP_PERCENT, TK_OP_PERCENT_EQ);
  if (ch == '+') return this->_opEq2('+', TK_OP_PLUS, TK_OP_PLUS_EQ, TK_OP_PLUS_PLUS);
  if (ch == '}') return this->_tok(TK_OP_RBRACE);
  if (ch == ']') return this->_tok(TK_OP_RBRACK);
  if (ch == ')') return this->_tok(TK_OP_RPAR);
  if (ch == ';') return this->_tok(TK_OP_SEMI);
  if (ch == '/') return this->_opEq(TK_OP_SLASH, TK_OP_SLASH_EQ);
  if (ch == '*') return this->_opEq2('*', TK_OP_STAR, TK_OP_STAR_EQ, TK_OP_STAR_STAR, TK_OP_STAR_STAR_EQ);
  if (ch == '~') return this->_tok(TK_OP_TILDE);

  if (ch == '.') {
    if (this->_reader->eof()) {
      return this->_tok(TK_OP_DOT);
    }

    auto loc1 = this->_reader->loc;
    auto ch1 = this->_reader->next();

    if (ch1 == '.' && !this->_reader->eof()) {
      auto ch2 = this->_reader->next();

      if (ch2 == '.') {
        this->val += ch1;
        this->val += ch2;

        return this->_tok(TK_OP_DOT_DOT_DOT);
      }
    }

    this->_reader->seek(loc1);
    return this->_tok(TK_OP_DOT);
  }

  if (ch == '?') {
    if (this->_reader->eof()) {
      return this->_tok(TK_OP_QN);
    }

    auto loc1 = this->_reader->loc;
    auto ch1 = this->_reader->next();

    if (ch1 == '.') {
      this->val += ch1;
      return this->_tok(TK_OP_QN_DOT);
    } else if (ch1 == '?') {
      this->val += ch1;

      if (this->_reader->eof()) {
        return this->_tok(TK_OP_QN_QN);
      }

      auto loc2 = this->_reader->loc;
      auto ch2 = this->_reader->next();

      if (ch2 == '=') {
        this->val += ch2;
        return this->_tok(TK_OP_QN_QN_EQ);
      } else {
        this->_reader->seek(loc2);
        return this->_tok(TK_OP_QN_QN);
      }
    } else {
      this->_reader->seek(loc1);
      return this->_tok(TK_OP_QN);
    }
  }

  if (Token::isIdStart(ch)) {
    this->_walk(Token::isIdContinue);

    if (this->val == "as") return this->_tok(TK_KW_AS);
    if (this->val == "async") return this->_tok(TK_KW_ASYNC);
    if (this->val == "await") return this->_tok(TK_KW_AWAIT);
    if (this->val == "break") return this->_tok(TK_KW_BREAK);
    if (this->val == "case") return this->_tok(TK_KW_CASE);
    if (this->val == "catch") return this->_tok(TK_KW_CATCH);
    if (this->val == "class") return this->_tok(TK_KW_CLASS);
    if (this->val == "const") return this->_tok(TK_KW_CONST);
    if (this->val == "continue") return this->_tok(TK_KW_CONTINUE);
    if (this->val == "default") return this->_tok(TK_KW_DEFAULT);
    if (this->val == "deinit") return this->_tok(TK_KW_DEINIT);
    if (this->val == "elif") return this->_tok(TK_KW_ELIF);
    if (this->val == "else") return this->_tok(TK_KW_ELSE);
    if (this->val == "enum") return this->_tok(TK_KW_ENUM);
    if (this->val == "export") return this->_tok(TK_KW_EXPORT);
    if (this->val == "fallthrough") return this->_tok(TK_KW_FALLTHROUGH);
    if (this->val == "false") return this->_tok(TK_KW_FALSE);
    if (this->val == "fn") return this->_tok(TK_KW_FN);
    if (this->val == "from") return this->_tok(TK_KW_FROM);
    if (this->val == "if") return this->_tok(TK_KW_IF);
    if (this->val == "import") return this->_tok(TK_KW_IMPORT);
    if (this->val == "in") return this->_tok(TK_KW_IN);
    if (this->val == "init") return this->_tok(TK_KW_INIT);
    if (this->val == "interface") return this->_tok(TK_KW_INTERFACE);
    if (this->val == "is") return this->_tok(TK_KW_IS);
    if (this->val == "loop") return this->_tok(TK_KW_LOOP);
    if (this->val == "main") return this->_tok(TK_KW_MAIN);
    if (this->val == "match") return this->_tok(TK_KW_MATCH);
    if (this->val == "mut") return this->_tok(TK_KW_MUT);
    if (this->val == "new") return this->_tok(TK_KW_NEW);
    if (this->val == "nil") return this->_tok(TK_KW_NIL);
    if (this->val == "obj") return this->_tok(TK_KW_OBJ);
    if (this->val == "op") return this->_tok(TK_KW_OP);
    if (this->val == "override") return this->_tok(TK_KW_OVERRIDE);
    if (this->val == "priv") return this->_tok(TK_KW_PRIV);
    if (this->val == "prot") return this->_tok(TK_KW_PROT);
    if (this->val == "pub") return this->_tok(TK_KW_PUB);
    if (this->val == "return") return this->_tok(TK_KW_RETURN);
    if (this->val == "static") return this->_tok(TK_KW_STATIC);
    if (this->val == "super") return this->_tok(TK_KW_SUPER);
    if (this->val == "this") return this->_tok(TK_KW_THIS);
    if (this->val == "throw") return this->_tok(TK_KW_THROW);
    if (this->val == "true") return this->_tok(TK_KW_TRUE);
    if (this->val == "try") return this->_tok(TK_KW_TRY);
    if (this->val == "type") return this->_tok(TK_KW_TYPE);
    if (this->val == "union") return this->_tok(TK_KW_UNION);

    return this->_tok(TK_ID);
  }

  if (Token::isDigit(ch)) {
    if (ch == '0') {
      if (this->_reader->eof()) {
        return this->_tok(TK_LIT_INT_DEC);
      }

      auto loc1 = this->_reader->loc;
      auto ch1 = this->_reader->next();

      if (Token::isDigit(ch1)) {
        this->_walk(Token::isIdContinue);
        throw LexerError(this, E0007);
      }

      if (ch1 == 'B' || ch1 == 'b' || ch1 == 'X' || ch1 == 'x' || ch1 == 'O' || ch1 == 'o') {
        this->val += ch1;

        if (ch1 == 'B' || ch1 == 'b') {
          return this->_litNum(Token::isLitIntBin, TK_LIT_INT_BIN);
        } else if (ch1 == 'X' || ch1 == 'x') {
          return this->_litNum(Token::isLitIntHex, TK_LIT_INT_HEX);
        } else {
          return this->_litNum(Token::isLitIntOct, TK_LIT_INT_OCT);
        }
      } else {
        this->_reader->seek(loc1);
      }

      return this->_litNum(Token::isLitIntDec, TK_LIT_INT_DEC);
    }

    this->_walk(Token::isLitIntDec);
    return this->_litNum(Token::isLitIntDec, TK_LIT_INT_DEC);
  }

  if (ch == '"') {
    while (true) {
      if (this->_reader->eof()) {
        throw LexerError(this, E0003);
      }

      auto loc1 = this->_reader->loc;
      auto ch1 = this->_reader->next();

      this->val += ch1;

      if (ch1 == '\\') {
        if (this->_reader->eof()) {
          throw LexerError(this, E0003);
        }

        auto ch2 = this->_reader->next();

        if (!Token::isLitStrEscape(ch2)) {
          this->loc = loc1;
          throw LexerError(this, E0005);
        }

        this->val += ch2;
      } else if (ch1 == '"') {
        break;
      }
    }

    return this->_tok(TK_LIT_STR);
  }

  if (ch == '\'') {
    if (this->_reader->eof()) {
      throw LexerError(this, E0002);
    }

    auto loc1 = this->_reader->loc;
    auto ch1 = this->_reader->next();

    if (ch1 == '\'') {
      throw LexerError(this, E0004);
    } else if (this->_reader->eof()) {
      throw LexerError(this, E0002);
    }

    this->val += ch1;

    if (ch1 == '\\') {
      auto ch2 = this->_reader->next();

      if (!Token::isLitCharEscape(ch2)) {
        this->loc = loc1;
        throw LexerError(this, E0005);
      } else if (this->_reader->eof()) {
        throw LexerError(this, E0002);
      }

      this->val += ch2;
    }

    auto ch3 = this->_reader->next();

    if (ch3 != '\'') {
      while (!this->_reader->eof()) {
        auto ch4 = this->_reader->next();

        if (ch4 == '\'') {
          break;
        }
      }

      throw LexerError(this, E0006);
    }

    this->val += ch3;
    return this->_tok(TK_LIT_CHAR);
  }

  throw LexerError(this, E0000);
}

Token Lexer::_litFloat (TokenType type) {
  if (!this->_reader->eof()) {
    auto loc1 = this->_reader->loc;
    auto ch1 = this->_reader->next();

    if (Token::isIdContinue(ch1)) {
      this->_walk(Token::isIdContinue);
      throw LexerError(this, E0012);
    }

    this->_reader->seek(loc1);
  }

  if (type == TK_LIT_INT_BIN) {
    throw LexerError(this, E0014);
  } else if (type == TK_LIT_INT_HEX) {
    throw LexerError(this, E0015);
  } else if (type == TK_LIT_INT_OCT) {
    throw LexerError(this, E0016);
  }

  return this->_tok(TK_LIT_FLOAT);
}

Token Lexer::_litNum (const std::function<bool (char)> &fn, TokenType type) {
  auto errCode = std::string();

  if (type == TK_LIT_INT_BIN) {
    errCode = E0008;
  } else if (type == TK_LIT_INT_DEC) {
    errCode = E0009;
  } else if (type == TK_LIT_INT_HEX) {
    errCode = E0010;
  } else {
    errCode = E0011;
  }

  if (type != TK_LIT_INT_DEC) {
    if (this->_reader->eof()) {
      throw LexerError(this, errCode);
    }

    auto ch1 = this->_reader->next();
    this->val += ch1;

    if (!fn(ch1)) {
      this->_walk(Token::isIdContinue);
      throw LexerError(this, errCode);
    }

    this->_walk(fn);
  }

  if (this->_reader->eof()) {
    return this->_tok(type);
  }

  auto loc2 = this->_reader->loc;
  auto ch2 = this->_reader->next();

  if (Token::isIdContinue(ch2) && ch2 != 'E' && ch2 != 'e') {
    this->_walk(Token::isIdContinue);
    throw LexerError(this, errCode);
  }

  if (ch2 == 'E' || ch2 == 'e') {
    this->val += ch2;
    this->_walkLitFloatExp(loc2);

    return this->_litFloat(type);
  } else if (ch2 == '.') {
    if (this->_reader->eof()) {
      throw LexerError(this, E0012);
    }

    auto loc3 = this->_reader->loc;
    auto ch3 = this->_reader->next();

    if (Token::isIdContinue(ch3) && !Token::isDigit(ch3) && ch3 != 'E' && ch3 != 'e') {
      this->_walk(Token::isIdContinue);
      throw LexerError(this, E0012);
    }

    if (ch3 == 'E' || ch3 == 'e') {
      this->val += ch2;
      this->val += ch3;

      this->_walkLitFloatExp(loc3);
      return this->_litFloat(type);
    } else if (ch3 == '.') {
      this->_reader->seek(loc2);
      return this->_tok(type);
    }

    this->val += ch2;
    this->val += ch3;

    while (!this->_reader->eof()) {
      auto loc4 = this->_reader->loc;
      auto ch4 = this->_reader->next();

      if (!Token::isDigit(ch4) && ch4 != 'E' && ch4 != 'e') {
        this->_reader->seek(loc4);
        break;
      }

      this->val += ch4;

      if (ch4 == 'E' || ch4 == 'e') {
        this->_walkLitFloatExp(loc4);
        break;
      }
    }

    return this->_litFloat(type);
  }

  this->_reader->seek(loc2);
  return this->_tok(type);
}

Token Lexer::_opEq (TokenType type1, TokenType type2) {
  if (this->_reader->eof()) {
    return this->_tok(type1);
  }

  auto loc1 = this->_reader->loc;
  auto ch1 = this->_reader->next();

  if (ch1 == '=') {
    this->val += ch1;
    return this->_tok(type2);
  }

  this->_reader->seek(loc1);
  return this->_tok(type1);
}

Token Lexer::_opEq2 (char ch, TokenType type1, TokenType type2, TokenType type3, const std::optional<TokenType> &type4) {
  if (this->_reader->eof()) {
    return this->_tok(type1);
  }

  auto loc1 = this->_reader->loc;
  auto ch1 = this->_reader->next();

  if (ch1 != '=' && ch1 != ch) {
    this->_reader->seek(loc1);
    return this->_tok(type1);
  } else if (ch1 == '=') {
    this->val += ch1;
    return this->_tok(type2);
  } else if (this->_reader->eof() || type4 == std::nullopt) {
    this->val += ch1;
    return this->_tok(type3);
  }

  auto loc2 = this->_reader->loc;
  auto ch2 = this->_reader->next();

  if (ch2 == '=') {
    this->val += ch1;
    this->val += ch2;

    return this->_tok(*type4);
  } else {
    this->val += ch1;
    this->_reader->seek(loc2);

    return this->_tok(type3);
  }
}

Token Lexer::_tok (TokenType type) {
  return Token{type, this->loc, this->_reader->loc, this->val};
}

void Lexer::_walk (const std::function<bool (char)> &fn) {
  while (!this->_reader->eof()) {
    auto loc1 = this->_reader->loc;
    auto ch1 = this->_reader->next();

    if (!fn(ch1)) {
      this->_reader->seek(loc1);
      break;
    }

    this->val += ch1;
  }
}

void Lexer::_walkLitFloatExp (ReaderLocation loc0) {
  if (this->_reader->eof()) {
    this->loc = loc0;
    throw LexerError(this, E0013);
  }

  auto ch1 = this->_reader->next();

  if (!Token::isDigit(ch1) && ch1 != '+' && ch1 != '-') {
    this->loc = loc0;
    this->_walk(Token::isIdContinue);

    throw LexerError(this, E0013);
  }

  this->val += ch1;

  if (ch1 == '+' || ch1 == '-') {
    if (this->_reader->eof()) {
      this->loc = loc0;
      throw LexerError(this, E0013);
    }

    auto ch2 = this->_reader->next();

    if (Token::isIdContinue(ch2) && !Token::isDigit(ch2)) {
      this->loc = loc0;
      this->_walk(Token::isIdContinue);

      throw LexerError(this, E0013);
    }

    this->val += ch2;
  }

  while (!this->_reader->eof()) {
    auto loc3 = this->_reader->loc;
    auto ch3 = this->_reader->next();

    if (!Token::isDigit(ch3)) {
      this->_reader->seek(loc3);
      break;
    }

    this->val += ch3;
  }
}

void Lexer::_whitespace () {
  if (this->_reader->eof()) {
    return;
  }

  auto loc1 = this->_reader->loc;
  auto ch1 = this->_reader->next();

  if (Token::isWhitespace(ch1)) {
    this->_walk(Token::isWhitespace);
    this->_whitespace();

    return;
  } else if (ch1 == '/' && !this->_reader->eof()) {
    auto ch2 = this->_reader->next();

    if (ch2 == '/') {
      this->val += ch2;

      while (!this->_reader->eof()) {
        auto loc3 = this->_reader->loc;
        auto ch3 = this->_reader->next();

        if (ch3 == '\n') {
          this->_reader->seek(loc3);
          break;
        }

        this->val += ch3;
      }

      this->_whitespace();
      return;
    } else if (ch2 == '*') {
      this->val += ch2;

      if (this->_reader->eof()) {
        throw LexerError(this, E0001);
      }

      while (true) {
        auto ch3 = this->_reader->next();
        this->val += ch3;

        if (this->_reader->eof()) {
          throw LexerError(this, E0001);
        }

        if (ch3 == '*') {
          auto loc4 = this->_reader->loc;
          auto ch4 = this->_reader->next();

          if (ch4 == '/') {
            this->val += ch4;
            break;
          } else {
            this->_reader->seek(loc4);
          }
        }
      }

      this->_whitespace();
      return;
    }
  }

  this->_reader->seek(loc1);
}

LexerError::LexerError (Lexer *lexer, const std::string &message) : Error("") {
  auto start = lexer->loc;
  auto end = lexer->_reader->loc;
  auto line = std::string();

  lexer->_reader->seek(ReaderLocation{start.pos - start.col, start.line});

  while (!lexer->_reader->eof()) {
    auto ch = lexer->_reader->next();

    if (ch == '\n') {
      break;
    }

    line += ch;
  }

  auto colNumStr = std::to_string(start.col + 1);
  auto lineNumStr = std::to_string(start.line);

  this->message_ += lexer->_reader->path + ':' + lineNumStr + ':' + colNumStr + ": " + message + "\n";
  this->message_ += "  " + lineNumStr + " | " + line + '\n';
  this->message_ += "  " + std::string(lineNumStr.length(), ' ') + " | " + std::string(start.col, ' ') + '^';

  auto underlineLen = static_cast<std::size_t>(0);

  if (start.line == end.line && line.length() > end.col) {
    underlineLen = end.col - start.col;
  } else if (line.length() > start.col) {
    underlineLen = line.length() - start.col;
  }

  if (underlineLen != 0) {
    this->message_ += std::string(underlineLen - 1, '~');
  }
}
