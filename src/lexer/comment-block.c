/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdlib.h>
#include <string.h>
#include "comment-block.h"

bool lex_comment_block (file_t *file, lexer_t *lexer, size_t pos) {
  unsigned char ch1 = file_readchar(file);
  unsigned char ch2;
  bool is_comment = false;

  if (ch1 == '/' && !file_eof(file)) {
    ch2 = file_readchar(file);

    if (ch2 == '*') {
      is_comment = true;
    }
  }

  if (!is_comment) {
    file_seek(file, pos);
    return false;
  }

  size_t len = 2;
  lexer->raw = malloc(len + 1);
  lexer->raw[len - 2] = ch1;
  lexer->raw[len - 1] = ch2;
  lexer->raw[len] = '\0';
  lexer->tok = LEXER_COMMENT_BLOCK;

  while (true) {
    if (file_eof(file)) {
      throw("SyntaxError: Unterminated block comment");
    }

    unsigned char ch = file_readchar(file);

    if (ch == '*' && !file_eof(file)) {
      size_t bu_pos = file_position(file);
      unsigned char ch_next = file_readchar(file);

      if (ch_next == '/') {
        len += 2;
        lexer->raw = realloc(lexer->raw, len + 1);
        lexer->raw[len - 2] = ch;
        lexer->raw[len - 1] = ch_next;
        lexer->raw[len] = '\0';

        break;
      } else {
        file_seek(file, bu_pos);
      }
    }

    lexer->raw = realloc(lexer->raw, ++len + 1);
    lexer->raw[len - 1] = ch;
    lexer->raw[len] = '\0';
  }

  lexer->str = malloc(len - 3);
  memcpy(lexer->str, lexer->raw + 2, len - 4);
  lexer->str[len - 4] = '\0';

  return true;
}
