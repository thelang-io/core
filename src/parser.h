/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef SRC_PARSER_H
#define SRC_PARSER_H

#include "array.h"
#include "lexer.h"

typedef struct parser_s parser_t;
typedef struct parser_arglist_s parser_arglist_t;
typedef struct parser_call_expr_s parser_call_expr_t;
typedef struct parser_expr_s parser_expr_t;
typedef struct parser_literal_s parser_literal_t;
typedef struct parser_ws_s parser_ws_t;

typedef enum {
  PARSER_ARGLIST,
  PARSER_CALL_EXPR,
  PARSER_EXPR,
  PARSER_ID,
  PARSER_LITERAL,
  PARSER_UNKNOWN,
  PARSER_WS
} parser_token;

struct parser_literal_s {
  lexer_t *lexer;
};

struct parser_expr_s {
  parser_id_t *id;
  parser_literal_t *literal;
  parser_token token;
};

struct parser_arglist_s {
  array_t *exprs;
};

struct parser_call_expr_s {
  parser_arglist_t *arglist;
  parser_id_t *id;
};

struct parser_ws_s {
  array_t *lexers;
};

struct parser_s {
  parser_token token;
};

void parser_free (parser_t *parser);
void parser_free_cb (void *it);
parser_t *parser_new (file_t *file);

void parser_arglist_free_ (parser_arglist_t *parser);
parser_arglist_t *parser_arglist_new_ (file_t *file);
void parser_call_expr_free_ (parser_call_expr_t *parser);
parser_call_expr_t *parser_call_expr_new_ (file_t *file);
void parser_expr_free_ (parser_expr_t *parser);
void parser_expr_free_cb_ (void *it);
parser_expr_t *parser_expr_new_ (file_t *file);
void parser_literal_free_ (parser_literal_t *parser);
parser_literal_t *parser_literal_new_ (file_t *file);
void parser_ws_free_ (parser_ws_t *parser);
parser_ws_t *parser_ws_new_ (file_t *file, bool alloc);

#endif
