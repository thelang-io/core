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

#ifndef SRC_CONFIG_HPP
#define SRC_CONFIG_HPP

#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)
  #define EOL "\r\n"
  #define ESC_EOL "\\r\\n"
  #define OS_FILE_EXT ".exe"
  #define OS_PATH_SEP "\\"
  #define OS_WINDOWS
#else
  #if defined(__APPLE__)
    #define OS_MACOS
  #elif defined(__linux__)
    #define OS_LINUX
  #endif

  #define EOL "\n"
  #define ESC_EOL "\\n"
  #define OS_FILE_EXT ".out"
  #define OS_PATH_SEP "/"
#endif

#endif
