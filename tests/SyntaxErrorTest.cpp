/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <gmock/gmock.h>

#include "../src/SyntaxError.hpp"
#include "ReaderMock.hpp"

TEST(SyntaxErrorTest, GenerateMessageStart) {
  auto reader = ::testing::StrictMock<MockReader>();

  EXPECT_CALL(reader, seek(ReaderLocation{0, 1, 0}))
    .Times(2);

  EXPECT_CALL(reader, eof())
    .Times(2)
    .WillRepeatedly(::testing::Return(false));

  EXPECT_CALL(reader, next())
    .Times(2)
    .WillOnce(::testing::Return('@'))
    .WillOnce(::testing::Return('\n'));

  EXPECT_CALL(reader, path())
    .Times(1)
    .WillOnce(::testing::Return("/tmp/test.out"));

  auto err = SyntaxError(&reader, {0, 1, 0}, "Inside test");

  EXPECT_STREQ(
    err.what(),
    "/tmp/test.out:1:1: Inside test\n  1 | @\n    | ^\n"
  );
}

TEST(SyntaxErrorTest, GenerateMessageContinue) {
  auto reader = ::testing::StrictMock<MockReader>();

  EXPECT_CALL(reader, seek(ReaderLocation{11, 11, 0}))
    .Times(1);

  EXPECT_CALL(reader, eof())
    .Times(10)
    .WillRepeatedly(::testing::Return(false));

  EXPECT_CALL(reader, next())
    .Times(10)
    .WillOnce(::testing::Return(' '))
    .WillOnce(::testing::Return(' '))
    .WillOnce(::testing::Return('@'))
    .WillOnce(::testing::Return('m'))
    .WillOnce(::testing::Return('a'))
    .WillOnce(::testing::Return('i'))
    .WillOnce(::testing::Return('n'))
    .WillOnce(::testing::Return(' '))
    .WillOnce(::testing::Return('{'))
    .WillOnce(::testing::Return('\n'));

  EXPECT_CALL(reader, seek(ReaderLocation{13, 11, 2}))
    .Times(1);

  EXPECT_CALL(reader, path())
    .Times(1)
    .WillOnce(::testing::Return("/tmp/test.out"));

  auto err = SyntaxError(&reader, {13, 11, 2}, "Inside test");

  EXPECT_STREQ(
    err.what(),
    "/tmp/test.out:11:3: Inside test\n  11 |   @main {\n     |   ^\n"
  );
}
