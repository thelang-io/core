/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdlib.h>
#include <string.h>
#include "op.h"

bool lex_op (duc_file_t *file, lexer_t *lexer, size_t pos) {
  unsigned char ch1 = duc_file_readchar(file);
  unsigned char ch2 = 0;
  unsigned char ch3 = 0;
  size_t len = 1;

  switch (ch1) {
    case '&': {
      LEX_OP_EQ2('&', LEXER_OP_AND, LEXER_OP_ANDEQ, LEXER_OP_ANDAND, LEXER_OP_ANDANDEQ);
      break;
    }
    case '^': {
      LEX_OP_EQ(LEXER_OP_CARET, LEXER_OP_CARETEQ);
      break;
    }
    case ':': {
      LEX_OP_EQ(LEXER_OP_COLON, LEXER_OP_COLONEQ);
      break;
    }
    case ',': {
      lexer->token = LEXER_OP_COMMA;
      break;
    }
    case '.': {
      if (!duc_file_eof(file)) {
        size_t bu_pos2 = duc_file_position(file);
        ch2 = duc_file_readchar(file);

        if (strchr("0123456789", ch2) != NULL) {
          duc_file_seek(file, pos);
          return false;
        } else if (ch2 != '.') {
          duc_file_seek(file, bu_pos2);
          lexer->token = LEXER_OP_DOT;
        } else if (!duc_file_eof(file)) {
          size_t bu_pos3 = duc_file_position(file);
          ch3 = duc_file_readchar(file);

          if (ch3 != '.' && ch3 != '=') {
            duc_file_seek(file, bu_pos3);
            lexer->token = LEXER_OP_DOTDOT;
            len += 1;
          } else if (ch3 == '=') {
            duc_file_seek(file, bu_pos3);
            lexer->token = LEXER_OP_DOTDOTEQ;
            len += 2;
          } else {
            lexer->token = LEXER_OP_DOTDOTDOT;
            len += 2;
          }
        } else {
          lexer->token = LEXER_OP_DOTDOT;
          len += 1;
        }
      } else {
        lexer->token = LEXER_OP_DOT;
      }

      break;
    }
    case '=': {
      LEX_OP_EQ(LEXER_OP_EQ, LEXER_OP_EQEQ);
      break;
    }
    case '!': {
      LEX_OP_EQ_BEFORE('!', LEXER_OP_EXCL, LEXER_OP_EXCLEQ, LEXER_OP_EXCLEXCL);
      break;
    }
    case '>': {
      LEX_OP_EQ2('>', LEXER_OP_GT, LEXER_OP_GTEQ, LEXER_OP_RSHIFT, LEXER_OP_RSHIFTEQ);
      break;
    }
    case '{': {
      lexer->token = LEXER_OP_LBRACE;
      break;
    }
    case '[': {
      lexer->token = LEXER_OP_LBRACK;
      break;
    }
    case '(': {
      lexer->token = LEXER_OP_LPAR;
      break;
    }
    case '<': {
      LEX_OP_EQ2('<', LEXER_OP_LT, LEXER_OP_LTEQ, LEXER_OP_LSHIFT, LEXER_OP_LSHIFTEQ);
      break;
    }
    case '-': {
      LEX_OP_EQ_BEFORE('-', LEXER_OP_MINUS, LEXER_OP_MINUSEQ, LEXER_OP_MINUSMINUS);
      break;
    }
    case '|': {
      LEX_OP_EQ2('|', LEXER_OP_OR, LEXER_OP_OREQ, LEXER_OP_OROR, LEXER_OP_OROREQ);
      break;
    }
    case '%': {
      LEX_OP_EQ(LEXER_OP_PERCENT, LEXER_OP_PERCENTEQ);
      break;
    }
    case '+': {
      LEX_OP_EQ_BEFORE('+', LEXER_OP_PLUS, LEXER_OP_PLUSEQ, LEXER_OP_PLUSPLUS);
      break;
    }
    case '?': {
      if (!duc_file_eof(file)) {
        size_t bu_pos2 = duc_file_position(file);
        ch2 = duc_file_readchar(file);

        if (ch2 != '?' && ch2 != '.') {
          duc_file_seek(file, bu_pos2);
          lexer->token = LEXER_OP_QN;
        } else if (ch2 == '.') {
          lexer->token = LEXER_OP_QNDOT;
          len += 1;
        } else if (!duc_file_eof(file)) {
          size_t bu_pos3 = duc_file_position(file);
          ch3 = duc_file_readchar(file);

          if (ch3 != '=') {
            duc_file_seek(file, bu_pos3);
            lexer->token = LEXER_OP_QNQN;
            len += 1;
          } else {
            lexer->token = LEXER_OP_QNQNEQ;
            len += 2;
          }
        } else {
          lexer->token = LEXER_OP_QNQN;
          len += 1;
        }
      } else {
        lexer->token = LEXER_OP_QN;
      }

      break;
    }
    case '}': {
      lexer->token = LEXER_OP_RBRACE;
      break;
    }
    case ']': {
      lexer->token = LEXER_OP_RBRACK;
      break;
    }
    case ')': {
      lexer->token = LEXER_OP_RPAR;
      break;
    }
    case ';': {
      lexer->token = LEXER_OP_SEMI;
      break;
    }
    case '/': {
      LEX_OP_EQ(LEXER_OP_SLASH, LEXER_OP_SLASHEQ);
      break;
    }
    case '*': {
      LEX_OP_EQ2('*', LEXER_OP_STAR, LEXER_OP_STAREQ, LEXER_OP_STARSTAR, LEXER_OP_STARSTAREQ);
      break;
    }
    case '~': {
      lexer->token = LEXER_OP_TILDE;
      break;
    }
    default: {
      duc_file_seek(file, pos);
      return false;
    }
  }

  lexer->raw = malloc(len + 1);

  switch (lexer->token) {
    case LEXER_OP_ANDANDEQ:
    case LEXER_OP_DOTDOTDOT:
    case LEXER_OP_DOTDOTEQ:
    case LEXER_OP_LSHIFTEQ:
    case LEXER_OP_OROREQ:
    case LEXER_OP_QNQNEQ:
    case LEXER_OP_RSHIFTEQ:
    case LEXER_OP_STARSTAREQ: {
      lexer->raw[len - 3] = ch1;
      lexer->raw[len - 2] = ch2;
      lexer->raw[len - 1] = ch3;
      lexer->raw[len] = '\0';
      break;
    }
    case LEXER_OP_ANDAND:
    case LEXER_OP_ANDEQ:
    case LEXER_OP_CARETEQ:
    case LEXER_OP_COLONEQ:
    case LEXER_OP_DOTDOT:
    case LEXER_OP_EQEQ:
    case LEXER_OP_EXCLEQ:
    case LEXER_OP_EXCLEXCL:
    case LEXER_OP_GTEQ:
    case LEXER_OP_LSHIFT:
    case LEXER_OP_LTEQ:
    case LEXER_OP_MINUSEQ:
    case LEXER_OP_MINUSMINUS:
    case LEXER_OP_OREQ:
    case LEXER_OP_OROR:
    case LEXER_OP_PERCENTEQ:
    case LEXER_OP_PLUSEQ:
    case LEXER_OP_PLUSPLUS:
    case LEXER_OP_QNDOT:
    case LEXER_OP_QNQN:
    case LEXER_OP_RSHIFT:
    case LEXER_OP_SLASHEQ:
    case LEXER_OP_STAREQ:
    case LEXER_OP_STARSTAR: {
      lexer->raw[len - 2] = ch1;
      lexer->raw[len - 1] = ch2;
      lexer->raw[len] = '\0';
      break;
    }
    default: {
      lexer->raw[len - 1] = ch1;
      lexer->raw[len] = '\0';
    }
  }

  lexer->str = malloc(len + 1);
  memcpy(lexer->str, lexer->raw, len + 1);

  return true;
}
