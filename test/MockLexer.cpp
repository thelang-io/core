/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "MockLexer.hpp"
#include "MockReader.hpp"

MockLexer::MockLexer (const std::string &content) : Lexer(nullptr), _r(testing::NiceMock<MockReader>(content)) {
  this->reader = &this->_r;

  ON_CALL(*this, next).WillByDefault([this] () -> Token {
    return this->Lexer::next();
  });

  ON_CALL(*this, seek).WillByDefault([this] (ReaderLocation l) -> void {
    return this->Lexer::seek(l);
  });

  ON_CALL(*this, whitespace).WillByDefault([this] () -> void {
    return this->Lexer::whitespace();
  });
}

MockLexer::~MockLexer () {
}
