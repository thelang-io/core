/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "Token.hpp"

static const char *token_type[] = {
  #define GEN_TOKEN_STR(x) #x,
  FOREACH_TOKEN(GEN_TOKEN_STR)
  #undef GEN_TOKEN_STR
};

bool Token::isIdContinue (const char ch) {
  const auto chs = std::string(
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_"
  );

  return chs.find(ch) != std::string::npos;
}

bool Token::isIdStart (const char ch) {
  const auto chs = std::string(
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"
  );

  return chs.find(ch) != std::string::npos;
}

bool Token::isWhitespace (const char ch) {
  return std::string("\r\n\t ").find(ch) != std::string::npos;
}

Token::Token (
  const TokenType &t,
  const std::string &v,
  const ReaderLocation &s,
  const ReaderLocation &e
) : type(t), val(v), start(s), end(e) {
}

bool Token::operator== (const Token &rhs) const {
  return this->type == rhs.type;
}

bool Token::operator!= (const Token &rhs) const {
  return !(*this == rhs);
}

bool Token::operator== (const TokenType &rhs) const {
  return this->type == rhs;
}

bool Token::operator!= (const TokenType &rhs) const {
  return !(*this == rhs);
}

std::string Token::str () const {
  return std::string(token_type[this->type]) + '(' +
    std::to_string(this->start.line) + ':' +
    std::to_string(this->start.col) + '-' +
    std::to_string(this->end.line) + ':' +
    std::to_string(this->end.col) + ')';
}
