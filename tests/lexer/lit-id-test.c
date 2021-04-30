/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <duc/testing.h>
#include "../../src/lexer/lit-id.h"
#include "../lexer-test.h"

#define LEX_LIT_KW_F(text, tok) \
  LEX_FS(text, tok); \
  LEX_FS(text "s", LEXER_LIT_ID)

DUC_TEST(lex_lit_id, works) {
  LEX_FS("a", LEXER_LIT_ID);
  LEX_FS("A_b1", LEXER_LIT_ID);
  LEX_FS("_Ab1", LEXER_LIT_ID);

  LEX_LIT_KW_F("as", LEXER_KW_AS);
  LEX_FS("as?", LEXER_KW_AS_SAFE);
  LEX_LIT_KW_F("case", LEXER_KW_CASE);
  LEX_LIT_KW_F("elif", LEXER_KW_ELIF);
  LEX_LIT_KW_F("else", LEXER_KW_ELSE);
  LEX_LIT_KW_F("enum", LEXER_KW_ENUM);
  LEX_LIT_KW_F("fn", LEXER_KW_FN);
  LEX_LIT_KW_F("from", LEXER_KW_FROM);
  LEX_LIT_KW_F("if", LEXER_KW_IF);
  LEX_LIT_KW_F("in", LEXER_KW_IN);
  LEX_LIT_KW_F("init", LEXER_KW_INIT);
  LEX_LIT_KW_F("is", LEXER_KW_IS);
  LEX_LIT_KW_F("loop", LEXER_KW_LOOP);
  LEX_LIT_KW_F("main", LEXER_KW_MAIN);
  LEX_LIT_KW_F("mut", LEXER_KW_MUT);
  LEX_LIT_KW_F("new", LEXER_KW_NEW);
  LEX_LIT_KW_F("nil", LEXER_KW_NIL);
  LEX_LIT_KW_F("obj", LEXER_KW_OBJ);
  LEX_LIT_KW_F("op", LEXER_KW_OP);
  LEX_LIT_KW_F("pub", LEXER_KW_PUB);
  LEX_LIT_KW_F("return", LEXER_KW_RETURN);
  LEX_LIT_KW_F("try", LEXER_KW_TRY);
}

DUC_TEST(lexer_lit_id, is_char_and_is_char_start) {
  DUC_ASSERT_TRUE(lexer_lit_id_is_char('A'));
  DUC_ASSERT_TRUE(lexer_lit_id_is_char('a'));
  DUC_ASSERT_TRUE(lexer_lit_id_is_char('0'));
  DUC_ASSERT_TRUE(lexer_lit_id_is_char('9'));
  DUC_ASSERT_TRUE(lexer_lit_id_is_char('_'));
  DUC_ASSERT_FALSE(lexer_lit_id_is_char('@'));

  DUC_ASSERT_TRUE(lexer_lit_id_is_char_start('A'));
  DUC_ASSERT_TRUE(lexer_lit_id_is_char_start('a'));
  DUC_ASSERT_TRUE(lexer_lit_id_is_char_start('_'));
  DUC_ASSERT_FALSE(lexer_lit_id_is_char_start('0'));
  DUC_ASSERT_FALSE(lexer_lit_id_is_char_start('9'));
  DUC_ASSERT_FALSE(lexer_lit_id_is_char_start('@'));
}

int main () {
  DUC_TEST_RUN(lex_lit_id, works);
  DUC_TEST_RUN(lexer_lit_id, is_char_and_is_char_start);
}
