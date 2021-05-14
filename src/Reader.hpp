/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef SRC_READER_HPP
#define SRC_READER_HPP

#include <filesystem>

namespace fs = std::filesystem;

struct ReaderLocation {
  size_t col = 1;
  size_t line = 1;
  size_t pos = 0;
};

class Reader {
 public:
  Reader (const fs::path &path);

  std::string content () const;
  bool eof () const;
  ReaderLocation loc () const;
  char next ();
  fs::path path () const;
  void seek (const ReaderLocation &loc);

 private:
  std::string _content;
  ReaderLocation _loc;
  fs::path _path;
};

bool operator== (const ReaderLocation &lhs, const ReaderLocation &rhs);

#endif
