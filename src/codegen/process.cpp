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

#include "process.hpp"
#include "../config.hpp"

const std::vector<std::string> codegenProcess = {
  R"(struct _{array_str} process_args (int argc, char **argv) {)" EOL
  R"(  _{struct str} *d = _{alloc}(argc * sizeof(_{struct str}));)" EOL
  R"(  for (int i = 0; i < argc; i++) d[i] = _{str_alloc}(argv[i]);)" EOL
  R"(  return (struct _{array_str}) {d, (_{size_t}) argc};)" EOL
  R"(})" EOL,

  R"(_{struct str} process_cwd () {)" EOL
  R"(  char buf[256];)" EOL
  R"(  #ifdef _{THE_OS_WINDOWS})" EOL
  R"(    char *p = _{_getcwd}(buf, 256);)" EOL
  R"(  #else)" EOL
  R"(    char *p = _{getcwd}(buf, 256);)" EOL
  R"(  #endif)" EOL
  R"(  if (p == _{NULL}) {)" EOL
  R"(    _{fprintf}(_{stderr}, "Error: failed to retrieve current working directory information" _{THE_EOL});)" EOL
  R"(    _{exit}(_{EXIT_FAILURE});)" EOL
  R"(  })" EOL
  R"(  return _{str_alloc}(buf);)" EOL
  R"(})" EOL,

  R"(_{int32_t} process_getgid () {)" EOL
  R"(  if (_{THE_OS_WINDOWS}) return 0;)" EOL
  R"(  return _{getgid}();)" EOL
  R"(})" EOL,

  R"(_{int32_t} process_getuid () {)" EOL
  R"(  if (_{THE_OS_WINDOWS}) return 0;)" EOL
  R"(  return _{getuid}();)" EOL
  R"(})" EOL,

  R"(_{struct buffer} process_runSync (_{struct str} s) {)" EOL
  R"(  char *c = _{alloc}(s.l + 1);)" EOL
  R"(  _{memcpy}(c, s.d, s.l);)" EOL
  R"(  c[s.l] = '\0';)" EOL
  R"(  #ifdef _{THE_OS_WINDOWS})" EOL
  R"(    _{FILE} *f = _{_popen}(c, "r");)" EOL
  R"(  #else)" EOL
  R"(    _{FILE} *f = _{popen}(c, "r");)" EOL
  R"(  #endif)" EOL
  R"(  if (f == _{NULL}) {)" EOL
  R"(    _{fprintf}(_{stderr}, "Error: failed to run process `%s`" _{THE_EOL}, c);)" EOL
  R"(    _{exit}(_{EXIT_FAILURE});)" EOL
  R"(  })" EOL
  R"(  unsigned char *d = _{NULL};)" EOL
  R"(  unsigned char b[4096];)" EOL
  R"(  _{size_t} l = 0;)" EOL
  R"(  _{size_t} y;)" EOL
  R"(  while ((y = _{fread}(b, 1, sizeof(b), f)) > 0) {)" EOL
  R"(    d = _{re_alloc}(d, l + y);)" EOL
  R"(    _{memcpy}(&d[l], b, y);)" EOL
  R"(    l += y;)" EOL
  R"(  })" EOL
  R"(  #ifdef _{THE_OS_WINDOWS})" EOL
  R"(    int e = _{_pclose}(f);)" EOL
  R"(  #else)" EOL
  R"(    int e = _{pclose}(f) / 256;)" EOL
  R"(  #endif)" EOL
  R"(  if (e != 0) {)" EOL
  R"(    _{fprintf}(_{stderr}, "Error: process `%s` exited with exit code %d" _{THE_EOL}, c, e);)" EOL
  R"(    _{exit}(_{EXIT_FAILURE});)" EOL
  R"(  })" EOL
  R"(  _{free}(c);)" EOL
  R"(  _{str_free}((_{struct str}) s);)" EOL
  R"(  return (_{struct buffer}) {d, l};)" EOL
  R"(})" EOL
};
