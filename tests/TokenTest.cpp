/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <gmock/gmock.h>
#include "../src/Token.hpp"

TEST(TokenTest, IsIdContinue) {
  EXPECT_TRUE(Token::isIdContinue('A'));
  EXPECT_TRUE(Token::isIdContinue('Z'));
  EXPECT_TRUE(Token::isIdContinue('a'));
  EXPECT_TRUE(Token::isIdContinue('z'));
  EXPECT_TRUE(Token::isIdContinue('0'));
  EXPECT_TRUE(Token::isIdContinue('9'));
  EXPECT_TRUE(Token::isIdContinue('_'));
}

TEST(TokenTest, IsIdStart) {
  EXPECT_TRUE(Token::isIdStart('A'));
  EXPECT_TRUE(Token::isIdStart('Z'));
  EXPECT_TRUE(Token::isIdStart('a'));
  EXPECT_TRUE(Token::isIdStart('z'));
  EXPECT_TRUE(Token::isIdStart('_'));
}

TEST(TokenTest, IsWhitespace) {
  EXPECT_TRUE(Token::isWhitespace('\r'));
  EXPECT_TRUE(Token::isWhitespace('\n'));
  EXPECT_TRUE(Token::isWhitespace('\t'));
  EXPECT_TRUE(Token::isWhitespace(' '));
}

TEST(TokenTest, CtorSetConstMembers) {
  const auto start = ReaderLocation{1, 1, 1};
  const auto end = ReaderLocation{2, 1, 2};
  const auto tok = Token(litId, "_", start, end);

  EXPECT_EQ(tok.end, end);
  EXPECT_EQ(tok.start, start);
  EXPECT_EQ(tok.type, litId);
  EXPECT_EQ(tok.val, "_");
}

TEST(TokenTest, EqualTokenType) {
  EXPECT_EQ(Token(litId, "_", {}, {}), litId);
}

TEST(TokenTest, NotEqualTokenType) {
  EXPECT_NE(Token(litId, "_", {}, {}), litFloat);
}

TEST(TokenTest, StrWithLocation) {
  const auto tok1 = Token(litId, "_", {1, 1, 1}, {2, 1, 2});
  EXPECT_EQ(tok1.str(), "litId(1:1-1:2)");
  const auto tok2 = Token(litId, "a_b", {14, 11, 3}, {17, 11, 6});
  EXPECT_EQ(tok2.str(), "litId(11:3-11:6)");
}
