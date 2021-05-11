/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "../src/lexer.h"
#include "lexer-test.h"

TEST(lexer, lookup) {
  char *filepath = "../test.out";

  writefile(filepath, " ");
  file_t *file = file_new(filepath, FILE_READ);
  ASSERT_EQ(lexer_lookup(file), LEXER_WS);
  file_free(file);

  writefile(filepath, "@");
  file = file_new(filepath, FILE_READ);
  ASSERT_EQ(lexer_lookup(file), LEXER_UNKNOWN);
  file_free(file);

  file_remove(filepath);
}

TEST(lexer, new_and_free) {
  char *filepath = "../test.out";

  writefile(filepath, " ");
  file_t *file = file_new(filepath, FILE_READ);
  lexer_t *lexer = lexer_new(file);

  ASSERT_NE(lexer, NULL);
  lexer_free(lexer);
  file_free(file);

  writefile(filepath, "@");
  file = file_new(filepath, FILE_READ);
  lexer = lexer_new(file);

  ASSERT_NE(lexer, NULL);
  ASSERT_EQ(lexer->raw, NULL);
  ASSERT_EQ(lexer->str, NULL);
  ASSERT_EQ(lexer->tok, LEXER_UNKNOWN);
  ASSERT_EQ(file_position(file), 0);

  lexer_free(lexer);
  file_free(file);

  file_remove(filepath);
}

int main () {
  TEST_RUN(lexer, lookup);
  TEST_RUN(lexer, new_and_free);
}
