/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

//#include <stdlib.h>
//#include <string.h>
#include "ast.h"

void ast_free (__attribute__((unused)) ast_t *ast) {
//  array_free(ast->parsers, parser_free_cb);
//  free(ast);
}

ast_t *ast_new (__attribute__((unused)) file_t *file) {
//  ast_t *ast = malloc(sizeof(ast_t));
//  ast->parsers = array_new();
//
//  while (!file_eof(file)) {
//    parser_t *parser = parser_new(file);
//
//    if (parser->tok == PARSER_UNKNOWN) {
//      throw("SyntaxError: Unexpected expression");
//    } else if (parser->tok == PARSER_WS) {
//      parser_free(parser);
//      continue;
//    }
//
//    array_push(ast->parsers, parser);
//  }
//
//  for (size_t i = 0, size = array_length(ast->parsers); i < size; i++) {
//    parser_t *parser = array_at(ast->parsers, i);
//
//    if (parser->tok == PARSER_CALL_EXPR) {
//      parser_call_expr_t *call_expr = parser->call_expr;
//
//      if (memcmp(call_expr->id->lexer->str, "print", 6) != 0) {
//        throw("SyntaxError: Unexpected call expression");
//      }
//
//      parser_arglist_t *arglist = call_expr->arglist;
//
//      for (size_t j = 0, size_arglist = array_length(arglist->exprs); j < size_arglist; j++) {
//        parser_expr_t *expr = array_at(arglist->exprs, j);
//
//        if (expr->tok != PARSER_LITERAL) {
//          throw("SyntaxError: Unexpected call expression argument");
//        }
//      }
//    }
//  }
//
//  return ast;
  return NULL;
}
