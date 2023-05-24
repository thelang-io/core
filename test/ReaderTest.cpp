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

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "../src/config.hpp"
#include "utils.hpp"

Reader readerTestGen (const std::string &path, const std::string &input) {
  auto f1 = std::ofstream(path);
  f1 << input;
  f1.close();
  return Reader(path);
}

class ReaderTest : public testing::Test {
 protected:
  Reader *r1_ = nullptr;
  Reader *r2_ = nullptr;
  Reader *r3_ = nullptr;

  void SetUp () override {
    auto emptyFile = std::ofstream("test_empty.txt");
    emptyFile.close();

    auto regularFile = std::ofstream("test_regular.txt");
    regularFile << "Hello, World!";
    regularFile.close();

    auto multilineFile = std::ofstream("test_multiline.txt");
    multilineFile << "Hello, Anna!" EOL "Hello, Nina!" EOL "Hello, Rosa!" EOL;
    multilineFile.close();

    this->r1_ = new Reader("test_empty.txt");
    this->r2_ = new Reader("test_regular.txt");
    this->r3_ = new Reader("test_multiline.txt");
  }

  void TearDown () override {
    delete this->r1_;
    delete this->r2_;
    delete this->r3_;

    std::filesystem::remove("test_empty.txt");
    std::filesystem::remove("test_regular.txt");
    std::filesystem::remove("test_multiline.txt");
  }
};

TEST_F(ReaderTest, ThrowsOnNotExisting) {
  EXPECT_THROW_WITH_MESSAGE({
    Reader("test.jpg");
  }, R"(no such file "test.jpg")");
}

TEST_F(ReaderTest, ThrowsOnDirectory) {
  EXPECT_THROW_WITH_MESSAGE({
    Reader("test");
  }, R"(path "test" is not a file)");
}

TEST_F(ReaderTest, ReadsFile) {
  EXPECT_EQ(this->r2_->loc, (ReaderLocation{0, 1, 0}));
  EXPECT_EQ(this->r2_->path, std::filesystem::current_path() / "test_regular.txt");
  EXPECT_EQ(this->r2_->content, "Hello, World!");
  EXPECT_EQ(this->r2_->size, 13);
}

TEST_F(ReaderTest, NoEofOnNonEmpty) {
  EXPECT_FALSE(this->r2_->eof());
}

TEST_F(ReaderTest, EofOnEmpty) {
  EXPECT_TRUE(this->r1_->eof());
}

TEST_F(ReaderTest, ReadsNext) {
  EXPECT_EQ(std::get<1>(this->r2_->next()), 'H');
  EXPECT_EQ(std::get<1>(this->r2_->next()), 'e');
  EXPECT_EQ(std::get<1>(this->r2_->next()), 'l');
  EXPECT_EQ(std::get<1>(this->r2_->next()), 'l');
  EXPECT_EQ(std::get<1>(this->r2_->next()), 'o');
}

TEST_F(ReaderTest, EofOnNext) {
  for (auto i = 0; i < 13; i++) {
    this->r2_->next();
  }

  EXPECT_TRUE(this->r2_->eof());
}

TEST_F(ReaderTest, Resets) {
  this->r2_->seek(ReaderLocation{7, 1, 7});
  this->r2_->reset();
  EXPECT_EQ(this->r2_->loc, (ReaderLocation{0, 1, 0}));
}

TEST_F(ReaderTest, SeeksTo) {
  this->r2_->seek(ReaderLocation{7, 1, 7});

  EXPECT_EQ(std::get<1>(this->r2_->next()), 'W');
  EXPECT_EQ(std::get<1>(this->r2_->next()), 'o');
  EXPECT_EQ(std::get<1>(this->r2_->next()), 'r');
  EXPECT_EQ(std::get<1>(this->r2_->next()), 'l');
  EXPECT_EQ(std::get<1>(this->r2_->next()), 'd');
  EXPECT_EQ(this->r2_->loc, (ReaderLocation{12, 1, 12}));
}

TEST_F(ReaderTest, EofOnSeek) {
  this->r2_->seek(ReaderLocation{12, 1, 12});
  EXPECT_FALSE(this->r2_->eof());
}

TEST_F(ReaderTest, ReadsMultiline) {
  while (!this->r3_->eof()) {
    this->r3_->next();
  }

  EXPECT_EQ(this->r3_->loc, (ReaderLocation{36 + std::string(EOL).size() * 3, 4, 0}));
}

TEST_F(ReaderTest, ThrowsOnNextOnEof) {
  EXPECT_THROW_WITH_MESSAGE({
    this->r1_->next();
  }, "tried to read on reader eof");
}

TEST_F(ReaderTest, ReadsShebang) {
  auto p = "test_shebang.txt";
  auto content = std::string("main {" EOL "  print(\"Hello, World!\")" EOL "}" EOL);

  EXPECT_EQ(readerTestGen(p, "#!/usr/bin/env the run").content, "");
  EXPECT_EQ(readerTestGen(p, "#!/usr/bin/env the run" EOL).content, "");
  EXPECT_EQ(readerTestGen(p, "#!/usr/bin/env the run" EOL + content).content, content);

  std::filesystem::remove(p);
}
