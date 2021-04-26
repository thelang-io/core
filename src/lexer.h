/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef SRC_LEXER_H
#define SRC_LEXER_H

#include <duc/fs.h>
#include <inttypes.h>
#include <stdint.h>

typedef struct lexer_s lexer_t;

typedef enum {
  LEXER_COLON,
  LEXER_COMMA,
  LEXER_FN,
  LEXER_ID,
  LEXER_IN,
  LEXER_LITINT_BIN,
  LEXER_LITINT_DEC,
  LEXER_LITINT_HEX,
  LEXER_LITINT_OCT,
  LEXER_LITSTR,
  LEXER_LBRACE,
  LEXER_LBRACK,
  LEXER_LOOP,
  LEXER_LPAR,
  LEXER_MAIN,
  LEXER_MUT,
  LEXER_RBRACE,
  LEXER_RBRACK,
  LEXER_RETURN,
  LEXER_RPAR,
  LEXER_UNKNOWN,
  LEXER_WS
} lexer_token;

struct lexer_s {
  uint8_t *raw;
  uint8_t *str;
  lexer_token token;
};

bool lexer_char_is_id (unsigned char ch);
bool lexer_char_is_id_start (unsigned char ch);
void lexer_free (lexer_t *lexer);
void lexer_free_cb (void *it);
lexer_t *lexer_new (duc_file_t *file);

bool lexer_is_bracket_ (duc_file_t *file, lexer_t *lexer, size_t pos);
bool lexer_is_id_ (duc_file_t *file, lexer_t *lexer, size_t pos);
bool lexer_is_keyword_ (duc_file_t *file, lexer_t *lexer, size_t pos);
bool lexer_is_litint_ (duc_file_t *file, lexer_t *lexer, size_t pos);
bool lexer_is_litstr_ (duc_file_t *file, lexer_t *lexer, size_t pos);
bool lexer_is_mark_ (duc_file_t *file, lexer_t *lexer, size_t pos);
bool lexer_is_ws_ (duc_file_t *file, lexer_t *lexer, size_t pos);

#endif
