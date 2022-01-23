/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef SRC_TOKEN_HPP
#define SRC_TOKEN_HPP

#include "Reader.hpp"

enum TokenType {
  TK_EOF,
  TK_ID,

  TK_KW_AS,
  TK_KW_ASYNC,
  TK_KW_AWAIT,
  TK_KW_BREAK,
  TK_KW_CASE,
  TK_KW_CATCH,
  TK_KW_CLASS,
  TK_KW_CONST,
  TK_KW_CONTINUE,
  TK_KW_DEFAULT,
  TK_KW_DEINIT,
  TK_KW_ELIF,
  TK_KW_ELSE,
  TK_KW_ENUM,
  TK_KW_EXPORT,
  TK_KW_FALLTHROUGH,
  TK_KW_FALSE,
  TK_KW_FN,
  TK_KW_FROM,
  TK_KW_IF,
  TK_KW_IMPORT,
  TK_KW_IN,
  TK_KW_INIT,
  TK_KW_INTERFACE,
  TK_KW_IS,
  TK_KW_LOOP,
  TK_KW_MAIN,
  TK_KW_MATCH,
  TK_KW_MUT,
  TK_KW_NEW,
  TK_KW_NIL,
  TK_KW_OBJ,
  TK_KW_OP,
  TK_KW_OVERRIDE,
  TK_KW_PRIV,
  TK_KW_PROT,
  TK_KW_PUB,
  TK_KW_RETURN,
  TK_KW_STATIC,
  TK_KW_SUPER,
  TK_KW_THIS,
  TK_KW_THROW,
  TK_KW_TRUE,
  TK_KW_TRY,
  TK_KW_TYPE,
  TK_KW_UNION,

  TK_LIT_CHAR,
  TK_LIT_FLOAT,
  TK_LIT_INT_BIN,
  TK_LIT_INT_DEC,
  TK_LIT_INT_HEX,
  TK_LIT_INT_OCT,
  TK_LIT_STR,

  TK_OP_AND,
  TK_OP_AND_AND,
  TK_OP_AND_AND_EQ,
  TK_OP_AND_EQ,
  TK_OP_CARET,
  TK_OP_CARET_EQ,
  TK_OP_COLON,
  TK_OP_COLON_EQ,
  TK_OP_COMMA,
  TK_OP_DOT,
  TK_OP_DOT_DOT_DOT,
  TK_OP_EQ,
  TK_OP_EQ_EQ,
  TK_OP_EXCL,
  TK_OP_EXCL_EQ,
  TK_OP_EXCL_EXCL,
  TK_OP_GT,
  TK_OP_GT_EQ,
  TK_OP_LBRACE,
  TK_OP_LBRACK,
  TK_OP_LPAR,
  TK_OP_LSHIFT,
  TK_OP_LSHIFT_EQ,
  TK_OP_LT,
  TK_OP_LT_EQ,
  TK_OP_MINUS,
  TK_OP_MINUS_EQ,
  TK_OP_MINUS_MINUS,
  TK_OP_OR,
  TK_OP_OR_EQ,
  TK_OP_OR_OR,
  TK_OP_OR_OR_EQ,
  TK_OP_PERCENT,
  TK_OP_PERCENT_EQ,
  TK_OP_PLUS,
  TK_OP_PLUS_EQ,
  TK_OP_PLUS_PLUS,
  TK_OP_QN,
  TK_OP_QN_DOT,
  TK_OP_QN_QN,
  TK_OP_QN_QN_EQ,
  TK_OP_RBRACE,
  TK_OP_RBRACK,
  TK_OP_RPAR,
  TK_OP_RSHIFT,
  TK_OP_RSHIFT_EQ,
  TK_OP_SEMI,
  TK_OP_SLASH,
  TK_OP_SLASH_EQ,
  TK_OP_STAR,
  TK_OP_STAR_EQ,
  TK_OP_STAR_STAR,
  TK_OP_STAR_STAR_EQ,
  TK_OP_TILDE
};

struct Token {
  TokenType type;
  ReaderLocation start;
  ReaderLocation end;
  std::string val;

  static bool isDigit (char);
  static bool isIdContinue (char);
  static bool isIdStart (char);
  static bool isLitCharEscape (char);
  static bool isLitIntBin (char);
  static bool isLitIntDec (char);
  static bool isLitIntHex (char);
  static bool isLitIntOct (char);
  static bool isLitStrEscape (char);
  static bool isWhitespace (char);

  int precedence () const;
  std::string str () const;
};

bool operator== (const Token &, const Token &);

#endif
