/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <gtest/gtest.h>
#include "../src/CodegenCleanUp.hpp"
#include "../src/config.hpp"

TEST(CodegenCleanUpTest, InitialSetValues) {
  auto n0 = CodegenCleanUp();
  auto n1 = CodegenCleanUp(1);

  EXPECT_EQ(n0.labelIdx, 0);
  EXPECT_EQ(n1.labelIdx, 1);
}

TEST(CodegenCleanUpTest, Adds) {
  auto n = CodegenCleanUp();
  n.add("test;");
  EXPECT_EQ(n.gen(0), "test;" EOL);
}

TEST(CodegenCleanUpTest, AddsToNonLabeled) {
  auto n = CodegenCleanUp();
  n.add("test1;");
  n.add("test2;");

  EXPECT_EQ(n.gen(0), "test2;" EOL "test1;" EOL);
}

TEST(CodegenCleanUpTest, AddsAfterLabelling) {
  auto n = CodegenCleanUp();
  n.add("test1;");
  n.add("test2;");
  auto l = n.currentLabel();
  n.add("test3;");

  EXPECT_EQ(n.gen(0), "test3;" EOL + l + ":" + EOL "test2;" EOL "test1;" EOL);
}

TEST(CodegenCleanUpTest, AddsAfterLabellingToNonLabeled) {
  auto n = CodegenCleanUp();
  n.add("test1;");
  n.add("test2;");
  auto l = n.currentLabel();
  n.add("test3;");
  n.add("test4;");

  EXPECT_EQ(n.gen(0), "test4;" EOL "test3;" EOL + l + ":" + EOL "test2;" EOL "test1;" EOL);
}

TEST(CodegenCleanUpTest, LabelsOnOneStatement) {
  auto n = CodegenCleanUp();
  n.add("test;");
  auto l = n.currentLabel();

  EXPECT_EQ(n.labelIdx, 1);
  EXPECT_EQ(n.gen(0), l + ":" + EOL "test;" EOL);
}

TEST(CodegenCleanUpTest, LabelsOnMultipleStatements) {
  auto n = CodegenCleanUp();
  n.add("test1;");
  n.add("test2;");
  n.add("test3;");
  auto l = n.currentLabel();

  EXPECT_EQ(n.labelIdx, 1);
  EXPECT_EQ(n.gen(0), l + ":" + EOL "test3;" EOL "test2;" EOL "test1;" EOL);
}

TEST(CodegenCleanUpTest, Empty) {
  EXPECT_TRUE(CodegenCleanUp().empty());
}

TEST(CodegenCleanUpTest, NonEmpty) {
  auto n1 = CodegenCleanUp();
  auto n2 = CodegenCleanUp();

  n1.add("test;");
  n2.add("test;");
  n2.currentLabel();

  EXPECT_FALSE(n1.empty());
  EXPECT_FALSE(n2.empty());
}

TEST(CodegenCleanUpTest, GeneratesOnEmpty) {
  EXPECT_EQ(CodegenCleanUp().gen(0), "");
}

TEST(CodegenCleanUpTest, GeneratesOnEmptyWithIndent) {
  EXPECT_EQ(CodegenCleanUp().gen(2), "");
}

TEST(CodegenCleanUpTest, GeneratesOnNonEmpty) {
  auto n = CodegenCleanUp();
  n.add("test;");
  EXPECT_EQ(n.gen(0), "test;" EOL);
}

TEST(CodegenCleanUpTest, GeneratesOnNonEmptyWithIndent) {
  auto n = CodegenCleanUp();
  n.add("test1;");
  n.add("test2;");
  n.add("test3;");

  EXPECT_EQ(n.gen(2), "  test3;" EOL"  test2;" EOL"  test1;" EOL);
}

TEST(CodegenCleanUpTest, GeneratesOnNonEmptyWithOneLabel) {
  auto n = CodegenCleanUp();
  n.add("test1;");
  n.add("test2;");
  n.add("test3;");
  auto l = n.currentLabel();

  EXPECT_EQ(n.gen(2), l + ":" EOL "  test3;" EOL "  test2;" EOL "  test1;" EOL);
}

TEST(CodegenCleanUpTest, GeneratesOnNonEmptyWithMultipleLabels) {
  auto n = CodegenCleanUp();
  n.add("test1;");
  n.add("test2;");
  n.add("test3;");
  auto l1 = n.currentLabel();
  n.add("test4;");
  n.add("test5;");
  n.add("test6;");
  auto l2 = n.currentLabel();
  n.add("test7;");
  n.add("test8;");
  n.add("test9;");
  auto l3 = n.currentLabel();

  EXPECT_EQ(
    n.gen(2),
    l3 + ":" EOL "  test9;" EOL "  test8;" EOL "  test7;" EOL +
    l2 + ":" EOL "  test6;" EOL "  test5;" EOL "  test4;" EOL +
    l1 + ":" EOL "  test3;" EOL "  test2;" EOL "  test1;" EOL
  );
}

TEST(CodegenCleanUpTest, UpdatesLabelIdx) {
  auto n = CodegenCleanUp();

  EXPECT_EQ(n.update(0).labelIdx, 0);
  EXPECT_EQ(n.update(2).labelIdx, 2);
  EXPECT_EQ(n.update(0).labelIdx, 0);
}
