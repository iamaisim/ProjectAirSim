// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "gtest/gtest.h"
#include "string_utils.hpp"

namespace projectairsim = microsoft::projectairsim;

TEST(StringUtils, LTrim) {
  std::string string("abc");
  projectairsim::StringUtils::LTrim(string);
  EXPECT_EQ(string, "abc");

  string = " abc";
  projectairsim::StringUtils::LTrim(string);
  EXPECT_EQ(string, "abc");

  string = "   abc";
  projectairsim::StringUtils::LTrim(string);
  EXPECT_EQ(string, "abc");

  string = "abc ";
  projectairsim::StringUtils::LTrim(string);
  EXPECT_EQ(string, "abc ");

  string = " abc ";
  projectairsim::StringUtils::LTrim(string);
  EXPECT_EQ(string, "abc ");
}

TEST(StringUtils, RTrim) {
  std::string string("abc");
  projectairsim::StringUtils::RTrim(string);
  EXPECT_EQ(string, "abc");

  string = "abc ";
  projectairsim::StringUtils::RTrim(string);
  EXPECT_EQ(string, "abc");

  string = "abc   ";
  projectairsim::StringUtils::RTrim(string);
  EXPECT_EQ(string, "abc");

  string = " abc";
  projectairsim::StringUtils::RTrim(string);
  EXPECT_EQ(string, " abc");

  string = " abc ";
  projectairsim::StringUtils::RTrim(string);
  EXPECT_EQ(string, " abc");
}

TEST(StringUtils, Trim) {
  std::string string("abc");
  projectairsim::StringUtils::Trim(string);
  EXPECT_EQ(string, "abc");

  string = "abc ";
  projectairsim::StringUtils::Trim(string);
  EXPECT_EQ(string, "abc");

  string = "abc   ";
  projectairsim::StringUtils::Trim(string);
  EXPECT_EQ(string, "abc");

  string = " abc";
  projectairsim::StringUtils::Trim(string);
  EXPECT_EQ(string, "abc");

  string = "   abc";
  projectairsim::StringUtils::Trim(string);
  EXPECT_EQ(string, "abc");

  string = " abc ";
  projectairsim::StringUtils::Trim(string);
  EXPECT_EQ(string, "abc");

  string = "   abc   ";
  projectairsim::StringUtils::Trim(string);
  EXPECT_EQ(string, "abc");
}

TEST(StringUtils, LTrimCopy) {
  EXPECT_EQ(projectairsim::StringUtils::LTrimCopy("abc"), "abc");
  EXPECT_EQ(projectairsim::StringUtils::LTrimCopy(" abc"), "abc");
  EXPECT_EQ(projectairsim::StringUtils::LTrimCopy("   abc"), "abc");
  EXPECT_EQ(projectairsim::StringUtils::LTrimCopy("abc "), "abc ");
  EXPECT_EQ(projectairsim::StringUtils::LTrimCopy(" abc "), "abc ");
}

TEST(StringUtils, RTrimCopy) {
  EXPECT_EQ(projectairsim::StringUtils::RTrimCopy("abc"), "abc");
  EXPECT_EQ(projectairsim::StringUtils::RTrimCopy("abc "), "abc");
  EXPECT_EQ(projectairsim::StringUtils::RTrimCopy("abc   "), "abc");
  EXPECT_EQ(projectairsim::StringUtils::RTrimCopy(" abc"), " abc");
  EXPECT_EQ(projectairsim::StringUtils::RTrimCopy(" abc "), " abc");
}

TEST(StringUtils, TrimCopy) {
  EXPECT_EQ(projectairsim::StringUtils::TrimCopy("abc"), "abc");
  EXPECT_EQ(projectairsim::StringUtils::TrimCopy("abc "), "abc");
  EXPECT_EQ(projectairsim::StringUtils::TrimCopy("abc   "), "abc");
  EXPECT_EQ(projectairsim::StringUtils::TrimCopy(" abc"), "abc");
  EXPECT_EQ(projectairsim::StringUtils::TrimCopy("   abc"), "abc");
  EXPECT_EQ(projectairsim::StringUtils::TrimCopy(" abc "), "abc");
  EXPECT_EQ(projectairsim::StringUtils::TrimCopy("   abc   "), "abc");
}
