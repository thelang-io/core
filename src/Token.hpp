/*!
 * Copyright (c) 2018 Aaron Delasy
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_TOKEN_HPP
#define SRC_TOKEN_HPP

#include "Reader.hpp"

enum TokenAssociativity {
  TK_ASSOC_NONE,
  TK_ASSOC_LEFT,
  TK_ASSOC_RIGHT
};

enum TokenType {
  TK_UNKNOWN,
  TK_EOF,
  TK_ID,
  TK_COMMENT_BLOCK,
  TK_COMMENT_LINE,

  TK_KW_ASYNC,
  TK_KW_AWAIT,
  TK_KW_BREAK,
  TK_KW_CATCH,
  TK_KW_CONST,
  TK_KW_CONTINUE,
  TK_KW_ELIF,
  TK_KW_ELSE,
  TK_KW_ENUM,
  TK_KW_FALSE,
  TK_KW_FN,
  TK_KW_FROM,
  TK_KW_IF,
  TK_KW_IS,
  TK_KW_LOOP,
  TK_KW_MAIN,
  TK_KW_MUT,
  TK_KW_NIL,
  TK_KW_OBJ,
  TK_KW_REF,
  TK_KW_RETURN,
  TK_KW_THROW,
  TK_KW_TRUE,
  TK_KW_TRY,
  TK_KW_TYPE,

  TK_LIT_CHAR,
  TK_LIT_FLOAT,
  TK_LIT_INT_BIN,
  TK_LIT_INT_DEC,
  TK_LIT_INT_HEX,
  TK_LIT_INT_OCT,
  TK_LIT_STR,

  TK_OP_AMP,
  TK_OP_AMP_EQ,
  TK_OP_AMP_AMP,
  TK_OP_AMP_AMP_EQ,
  TK_OP_ARROW,
  TK_OP_AT,
  TK_OP_BACKSLASH,
  TK_OP_BACKTICK,
  TK_OP_CARET,
  TK_OP_CARET_EQ,
  TK_OP_COLON,
  TK_OP_COLON_EQ,
  TK_OP_COMMA,
  TK_OP_DOLLAR,
  TK_OP_DOT,
  TK_OP_DOT_DOT_DOT,
  TK_OP_EQ,
  TK_OP_EQ_EQ,
  TK_OP_EXCL,
  TK_OP_EXCL_EQ,
  TK_OP_GT,
  TK_OP_GT_EQ,
  TK_OP_HASH,
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
  TK_OP_PIPE,
  TK_OP_PIPE_EQ,
  TK_OP_PIPE_PIPE,
  TK_OP_PIPE_PIPE_EQ,
  TK_OP_PERCENT,
  TK_OP_PERCENT_EQ,
  TK_OP_PLUS,
  TK_OP_PLUS_EQ,
  TK_OP_PLUS_PLUS,
  TK_OP_QN,
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
  TK_OP_TILDE
};

struct Token {
  TokenType type;
  std::string val = "";
  ReaderLocation start = {};
  ReaderLocation end = {};

  static std::string escape (const std::string &, bool = false);
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
  static std::string upper (const std::string &);
  static std::string upperFirst (const std::string &);

  TokenAssociativity associativity (bool = false) const;
  int precedence (bool = false) const;
  std::string str (std::size_t = 0) const;
  std::string xml (std::size_t = 0) const;
};

#endif
