/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef SRC_LEXER_LIT_STR_H
#define SRC_LEXER_LIT_STR_H

#include "../lexer.h"

bool lex_lit_str (duc_file_t *file, lexer_t *lexer, size_t pos);
void lex_lit_str_block_ (duc_file_t *file, lexer_t *lexer, size_t *len);

#endif
