/*!
 * Copyright (c) 2018 Aaron Delasy
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "error.hpp"
#include "../config.hpp"

const std::vector<std::string> codegenError = {
  R"~(void error_alloc (_{err_state_t} *fn_err_state, _{size_t} n1) {)~" EOL
  R"~(  char d[4096];)~" EOL
  R"~(  _{size_t} l = 0;)~" EOL
  R"~(  for (int i = fn_err_state->stack_idx - 1; i >= 0; i--) {)~" EOL
  R"~(    _{err_stack_t} it = fn_err_state->stack[i];)~" EOL
  R"~(    const char *fmt = _{THE_EOL} "  at %s (%s)";)~" EOL
  R"~(    _{size_t} z = _{snprintf}(_{NULL}, 0, fmt, it.name, it.file);)~" EOL
  R"~(    if (l + z >= 4096) {)~" EOL
  R"~(      break;)~" EOL
  R"~(    })~" EOL
  R"~(    _{sprintf}(&d[l], fmt, it.name, it.file);)~" EOL
  R"~(    l += z;)~" EOL
  R"~(  })~" EOL
  R"~(  _{fprintf}(_{stderr}, "Allocation Error: failed to allocate %zu bytes%s" _{THE_EOL}, n1, d);)~" EOL
  R"~(  _{exit}(_{EXIT_FAILURE});)~" EOL
  R"~(})~" EOL,

  R"(void error_assign (_{err_state_t} *fn_err_state, int id, void *ctx, void (*f) (void *), int line, int col) {)" EOL
  R"(  fn_err_state->id = id;)" EOL
  R"(  fn_err_state->ctx = ctx;)" EOL
  R"(  fn_err_state->_free = f;)" EOL
  R"(  _{error_stack_pos}(fn_err_state, line, col);)" EOL
  R"(  _{error_stack_str}(fn_err_state);)" EOL
  R"(})" EOL,

  R"(void error_stack_pop (_{err_state_t} *fn_err_state) {)" EOL
  R"(  fn_err_state->stack_idx--;)" EOL
  R"(})" EOL,

  R"(void error_stack_pos (_{err_state_t} *fn_err_state, int line, int col) {)" EOL
  R"(  if (line != 0) fn_err_state->stack[fn_err_state->stack_idx - 1].line = line;)" EOL
  R"(  if (col != 0) fn_err_state->stack[fn_err_state->stack_idx - 1].col = col;)" EOL
  R"(})" EOL,

  R"(void error_stack_push (_{err_state_t} *fn_err_state, const char *file, const char *name, int line, int col) {)" EOL
  R"(  fn_err_state->stack[fn_err_state->stack_idx].file = file;)" EOL
  R"(  fn_err_state->stack[fn_err_state->stack_idx].name = name;)" EOL
  R"(  _{error_stack_pos}(fn_err_state, line, col);)" EOL
  R"(  fn_err_state->stack_idx++;)" EOL
  R"(})" EOL,

  R"~(void error_stack_str (_{err_state_t} *fn_err_state) {)~" EOL
  R"~(  _{struct str} *stack = (_{struct str} *) &((struct _{error_Error} *) fn_err_state->ctx)->__THE_0_stack;)~" EOL
  R"~(  _{struct str} message = ((struct _{error_Error} *) fn_err_state->ctx)->__THE_0_message;)~" EOL
  R"~(  stack->l = message.l;)~" EOL
  R"~(  stack->d = _{re_alloc}(stack->d, stack->l);)~" EOL
  R"~(  _{memcpy}(stack->d, message.d, stack->l);)~" EOL
  R"~(  for (int i = fn_err_state->stack_idx - 1; i >= 0; i--) {)~" EOL
  R"~(    _{err_stack_t} it = fn_err_state->stack[i];)~" EOL
  R"~(    _{size_t} z;)~" EOL
  R"~(    char *fmt;)~" EOL
  R"~(    if (it.col == 0 && it.line == 0) {)~" EOL
  R"~(      fmt = _{THE_EOL} "  at %s (%s)";)~" EOL
  R"~(      z = _{snprintf}(_{NULL}, 0, fmt, it.name, it.file);)~" EOL
  R"~(    } else if (it.col == 0) {)~" EOL
  R"~(      fmt = _{THE_EOL} "  at %s (%s:%d)";)~" EOL
  R"~(      z = _{snprintf}(_{NULL}, 0, fmt, it.name, it.file, it.line);)~" EOL
  R"~(    } else {)~" EOL
  R"~(      fmt = _{THE_EOL} "  at %s (%s:%d:%d)";)~" EOL
  R"~(      z = _{snprintf}(_{NULL}, 0, fmt, it.name, it.file, it.line, it.col);)~" EOL
  R"~(    })~" EOL
  R"~(    stack->d = _{re_alloc}(stack->d, stack->l + z + 1);)~" EOL
  R"~(    if (it.col == 0 && it.line == 0) {)~" EOL
  R"~(      _{sprintf}(&stack->d[stack->l], fmt, it.name, it.file);)~" EOL
  R"~(    } else if (it.col == 0) {)~" EOL
  R"~(      _{sprintf}(&stack->d[stack->l], fmt, it.name, it.file, it.line);)~" EOL
  R"~(    } else {)~" EOL
  R"~(      _{sprintf}(&stack->d[stack->l], fmt, it.name, it.file, it.line, it.col);)~" EOL
  R"~(    })~" EOL
  R"~(    stack->l += z;)~" EOL
  R"~(  })~" EOL
  R"~(})~" EOL,

  R"(void error_unset (_{err_state_t} *fn_err_state) {)" EOL
  R"(  fn_err_state->id = -1;)" EOL
  R"(  fn_err_state->_free = _{NULL};)" EOL
  R"(})" EOL,

  R"(struct _{error_Error} *new_error (_{struct str} n1) {)" EOL
  R"(  return _{error_Error_alloc}(n1, (_{struct str}) {_{NULL}, 0});)" EOL
  R"(})" EOL
};
