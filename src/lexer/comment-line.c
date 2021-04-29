/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdlib.h>
#include <string.h>
#include "comment-line.h"

bool lex_comment_line (duc_file_t *file, lexer_t *lexer, size_t pos) {
  unsigned char ch1 = duc_file_readchar(file);
  unsigned char ch2;
  bool is_comment = false;

  if (ch1 == '/' && !duc_file_eof(file)) {
    ch2 = duc_file_readchar(file);

    if (ch2 == '/') {
      is_comment = true;
    }
  }

  if (!is_comment) {
    duc_file_seek(file, pos);
    return false;
  }

  size_t len = 2;
  lexer->raw = malloc(len + 1);
  lexer->raw[len - 2] = ch1;
  lexer->raw[len - 1] = ch2;
  lexer->raw[len] = '\0';
  lexer->token = LEXER_COMMENT_LINE;

  while (!duc_file_eof(file)) {
    size_t bu_pos = duc_file_position(file);
    unsigned char ch = duc_file_readchar(file);

    if (ch == '\n') {
      duc_file_seek(file, bu_pos);
      break;
    }

    lexer->raw = realloc(lexer->raw, ++len + 1);
    lexer->raw[len - 1] = ch;
    lexer->raw[len] = '\0';
  }

  lexer->str = malloc(len - 1);
  memcpy(lexer->str, lexer->raw + 2, len - 1);

  return true;
}
