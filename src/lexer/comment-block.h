/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef SRC_LEXER_COMMENT_BLOCK_H
#define SRC_LEXER_COMMENT_BLOCK_H

#include "../lexer.h"

bool lex_comment_block (duc_file_t *file, lexer_t *lexer, size_t pos);

#endif
