/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <duc/testing.h>
#include "../../src/lexer/op.h"
#include "../lexer-test.h"

DUC_TEST(lex_op, works) {
  LEX_FS(":", LEXER_OP_COLON);
  LEX_FS(",", LEXER_OP_COMMA);
  LEX_FS(".", LEXER_OP_DOT);
  LEX_FS("{", LEXER_OP_LBRACE);
  LEX_FS("[", LEXER_OP_LBRACK);
  LEX_FS("(", LEXER_OP_LPAR);
  LEX_FS("}", LEXER_OP_RBRACE);
  LEX_FS("]", LEXER_OP_RBRACK);
  LEX_FS(")", LEXER_OP_RPAR);
}

int main () {
  DUC_TEST_RUN(lex_op, works);
}
