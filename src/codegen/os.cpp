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

#include "os.hpp"
#include "../config.hpp"

const std::vector<std::string> codegenOS = {
  "_{struct str} os_name () {" EOL
  "  #ifdef _{THE_OS_WINDOWS}" EOL
  R"(    return _{str_alloc}("Windows");)" EOL
  "  #else" EOL
  "    _{struct utsname} buf;" EOL
  "    if (_{uname}(&buf) < 0) {" EOL
  R"(      _{fprintf}(_{stderr}, "Error: failed to retrieve uname information" _{THE_EOL});)" EOL
  "      _{exit}(_{EXIT_FAILURE});" EOL
  "    }" EOL
  R"(    if (_{strcmp}(buf.sysname, "Darwin") == 0) return _{str_alloc}("macOS");)" EOL
  "    return _{str_alloc}(buf.sysname);" EOL
  "  #endif" EOL
  "}" EOL
};
