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

#include "thread.hpp"
#include "../config.hpp"

const std::vector<std::string> codegenThread = {
  R"(void thread_sleep (_{int32_t} i) {)" EOL
  R"(  #ifdef _{THE_OS_WINDOWS})" EOL
  R"(    _{Sleep}((unsigned int) i);)" EOL
  R"(  #else)" EOL
  R"(    _{usleep}((unsigned int) (i * 1000));)" EOL
  R"(  #endif)" EOL
  R"(})" EOL
};
