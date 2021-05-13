/**
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef SRC_ERROR_HPP
#define SRC_ERROR_HPP

#include <string>

class Error : std::exception {
 public:
  explicit Error (const std::string &message);
  const char *what () const noexcept override;

 private:
  std::string _message;
};

#endif
