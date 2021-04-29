/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <duc/testing.h>
#include "../../src/lexer/lit-str.h"
#include "../lexer-test.h"

#define LEX_LIT_STR_F(text) \
  LEX_FE("\"" text "\"", text, LEXER_LIT_STR); \
  LEX_FE("\"Test" text "\"", "Test" text, LEXER_LIT_STR); \
  LEX_FE("\"" text "Test\"", text "Test", LEXER_LIT_STR); \
  LEX_FE("\"Test" text "Test\"", "Test" text "Test", LEXER_LIT_STR)

DUC_TEST(lex_lit_str, works) {
  LEX_FE("\"\"", "", LEXER_LIT_STR);
  LEX_FE("\"a\"", "a", LEXER_LIT_STR);
  LEX_FE("\"Test\"", "Test", LEXER_LIT_STR);
  LEX_FE("\"$test\"", "$test", LEXER_LIT_STR);
  LEX_FE("\"${test.name}\"", "${test.name}", LEXER_LIT_STR);

  LEX_LIT_STR_F("\\$");
  LEX_LIT_STR_F("\\$name");
  LEX_LIT_STR_F("\\0");
  LEX_LIT_STR_F("\\t");
  LEX_LIT_STR_F("\\n");
  LEX_LIT_STR_F("\\r");
  LEX_LIT_STR_F("\\\"");
  LEX_LIT_STR_F("\\'");
  LEX_LIT_STR_F("\\\\");
}

int main () {
  DUC_TEST_RUN(lex_lit_str, works);
}
