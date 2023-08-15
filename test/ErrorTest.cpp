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

#include <gtest/gtest.h>
#include "../src/Error.hpp"
#include "../src/Lexer.hpp"
#include "../src/config.hpp"
#include "MockReader.hpp"
#include "utils.hpp"

TEST(ErrorTest, ThrowsExactMessage) {
  EXPECT_THROW_WITH_MESSAGE({
    throw Error("Hello, World!");
  }, "Hello, World!");
}

TEST(LexerErrorTest, SingleToken) {
  auto reader = testing::NiceMock<MockReader>("@");
  auto lexer = Lexer(&reader);

  EXPECT_THROW_WITH_MESSAGE({
    throw Error(lexer.reader, lexer.loc, E0000);
  }, prepareTestOutput(std::string("/test:1:1: ") + E0000 + EOL "  1 | @" EOL "    | ^"));
}

TEST(LexerErrorTest, MultipleTokens) {
  auto reader = testing::NiceMock<MockReader>("test");
  auto lexer = Lexer(&reader);

  reader.loc = ReaderLocation{4, 1, 4};

  EXPECT_THROW_WITH_MESSAGE({
    throw Error(lexer.reader, lexer.loc, E0000);
  }, prepareTestOutput(std::string("/test:1:1: ") + E0000 + EOL "  1 | test" EOL "    | ^~~~"));
}

TEST(LexerErrorTest, MultipleTokensWithEnd) {
  auto reader = testing::NiceMock<MockReader>("test");
  auto lexer = Lexer(&reader);

  EXPECT_THROW_WITH_MESSAGE({
    throw Error(lexer.reader, lexer.loc, ReaderLocation{2, 1, 2}, E0000);
  }, prepareTestOutput(std::string("/test:1:1: ") + E0000 + EOL "  1 | test" EOL "    | ^~"));
}

TEST(LexerErrorTest, MultipleTokensAfterTokens) {
  auto reader = testing::NiceMock<MockReader>("1 + test");
  auto lexer = Lexer(&reader);

  lexer.loc = ReaderLocation{4, 1, 4};
  reader.loc = ReaderLocation{9, 1, 9};

  EXPECT_THROW_WITH_MESSAGE({
    throw Error(lexer.reader, lexer.loc, E0000);
  }, prepareTestOutput(std::string("/test:1:5: ") + E0000 + EOL "  1 | 1 + test" EOL "    |     ^~~~"));
}

TEST(LexerErrorTest, MultipleTokensAfterNewLine) {
  auto reader = testing::NiceMock<MockReader>("print()" EOL "/*Hello");
  auto lexer = Lexer(&reader);

  lexer.loc = ReaderLocation{7 + std::string(EOL).size(), 2, 0};
  reader.loc = ReaderLocation{14 + std::string(EOL).size(), 2, 7};

  EXPECT_THROW_WITH_MESSAGE({
    throw Error(lexer.reader, lexer.loc, E0001);
  }, prepareTestOutput(std::string("/test:2:1: ") + E0001 + EOL "  2 | /*Hello" EOL "    | ^~~~~~~"));
}

TEST(LexerErrorTest, MultipleTokensBetweenNewLines) {
  auto reader = testing::NiceMock<MockReader>("print()" EOL "/*Hello" EOL "Denis");
  auto lexer = Lexer(&reader);

  lexer.loc = ReaderLocation{7 + std::string(EOL).size(), 2, 0};
  reader.loc = ReaderLocation{19 + std::string(EOL).size() * 2, 3, 5};

  EXPECT_THROW_WITH_MESSAGE({
    throw Error(lexer.reader, lexer.loc, E0001);
  }, prepareTestOutput(std::string("/test:2:1: ") + E0001 + EOL "  2 | /*Hello" EOL "    | ^~~~~~~"));
}

TEST(LexerErrorTest, MultipleTokensBeforeNewLine) {
  auto reader = testing::NiceMock<MockReader>("/*Hello" EOL "Denis");
  auto lexer = Lexer(&reader);

  reader.loc = ReaderLocation{13, 2, 5};

  EXPECT_THROW_WITH_MESSAGE({
    throw Error(lexer.reader, lexer.loc, E0001);
  }, prepareTestOutput(std::string("/test:1:1: ") + E0001 + EOL "  1 | /*Hello" EOL "    | ^~~~~~~"));
}
