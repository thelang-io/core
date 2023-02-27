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

#include "MockLexer.hpp"

MockLexer::MockLexer (const std::string &content) : _r(content), Lexer(&this->_r) {
  ON_CALL(*this, next).WillByDefault([this] (bool _1) {
    return this->Lexer::next(_1);
  });

  ON_CALL(*this, seek).WillByDefault([this] (ReaderLocation l) {
    return this->Lexer::seek(l);
  });
}

MockLexer::~MockLexer () {
}
