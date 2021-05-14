/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "SyntaxError.hpp"

SyntaxError::SyntaxError (Reader *reader, const std::string &msg) {
  const auto loc = reader->loc();
  reader->seek({0, loc.line, loc.pos - loc.col});
  auto lineContent = std::string();

  while (!reader->eof()) {
    char ch = reader->next();

    if (ch == '\n') {
      break;
    }

    lineContent += ch;
  }

  reader->seek(loc);
  const auto colStr = std::to_string(loc.col);
  const auto lineStr = std::to_string(loc.line);

  this->message = reader->path().string() + ":" +
    lineStr + ":" + colStr + ": " + msg + "\n" +
    "  " + lineStr + " | " + lineContent + "\n" +
    "  " + std::string(lineStr.length(), ' ') +
    " | " + std::string(loc.col - 1, ' ') + "^\n";

  this->name = "SyntaxError";
}
