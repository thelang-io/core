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

/** @created_at: 2018-05-08 09:55 */

#include <filesystem>
#include <iostream>
#include "Codegen.hpp"
#include "utils.hpp"

int main (int argc, char *argv[]) {
  auto cwd = std::filesystem::current_path().string();

  try {
    if (argc == 1) {
      throw Error("REPL is not supported");
    }

    auto isAST = false;
    auto isCodegen = false;
    auto isDoc = false;
    auto isLex = false;
    auto isParse = false;
    auto fileName = std::optional<std::string>{};
    auto output = std::string("build/a.out");
    auto arch = std::string("");
    auto platform = std::string("default");

    for (auto i = 1; i < argc; i++) {
      auto arg = std::string(argv[i]);
      auto isOpt = arg.starts_with('-');

      if (i == 1 && arg == "ast") {
        isAST = true;
      } else if (i == 1 && arg == "codegen") {
        isCodegen = true;
      } else if (i == 1 && arg == "doc") {
        isDoc = true;
      } else if (i == 1 && arg == "lex") {
        isLex = true;
      } else if (i == 1 && arg == "parse") {
        isParse = true;
      } else if (isOpt) {
        auto shorthandOpt = !arg.starts_with("--");

        if (shorthandOpt) {
          throw Error("bad option " + arg);
        }

        auto optName = arg.substr(2, arg.find('=') - 2);
        auto optVal = arg.substr(arg.find('=') + 1);

        if (optName == "arch") {
          if (std::set<std::string>{"", "arm64", "x86_64"}.contains(optVal)) {
            arch = optVal;
          } else {
            throw Error("unsupported arch " + optVal);
          }
        } else if (optName == "output") {
          output = optVal;
        } else if (optName == "platform") {
          if (std::set<std::string>{"linux", "macos", "windows"}.contains(optVal)) {
            platform = optVal;
          } else {
            throw Error("unsupported platform " + optVal);
          }
        } else {
          throw Error("bad option " + arg);
        }
      } else if (fileName == std::nullopt) {
        fileName = arg;
      } else {
        throw Error("processing multiple files is not supported");
      }
    }

    if (fileName == std::nullopt) {
      throw Error("file is not specified");
    }

    auto reader = Reader(*fileName);
    auto lexer = Lexer(&reader);

    cwd = std::filesystem::path(reader.path).parent_path().string();

    if (isLex) {
      while (true) {
        auto [_, tok] = lexer.next();

        if (tok.type == TK_EOF) {
          break;
        }

        std::cout << tok.str() << std::endl;
      }

      return EXIT_SUCCESS;
    }

    auto parser = Parser(&lexer);

    if (isParse) {
      std::cout << parser.xml();
      return EXIT_SUCCESS;
    }

    auto ast = AST(&parser);

    if (isAST) {
      std::cout << ast.xml();
      return EXIT_SUCCESS;
    } else if (isDoc) {
      ast.gen();
      reader.reset();

      std::cout << parser.doc();
      return EXIT_SUCCESS;
    }

    auto codegen = Codegen(&ast);
    auto result = codegen.gen();

    if (isCodegen) {
      std::cout << std::get<0>(result);
      return EXIT_SUCCESS;
    }

    Codegen::compile(output, result, arch, platform);
    return EXIT_SUCCESS;
  } catch (const Error &err) {
    auto message = std::string(err.what());
    std::cerr << "Error: " << str_replace_all(message, cwd, ".") << std::endl;
    return EXIT_FAILURE;
  }
}
