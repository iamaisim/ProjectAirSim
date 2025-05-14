// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "jsonc.hpp"

#include <fstream>
#include <iostream>
#include <json.hpp>
#include <sstream>

#include "pch.h"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {
namespace client {

// This stream buffer class acts as a filter that removes the comments from
// a JSON Commented stream returning a plain JSON stream.
class jsoncbuf : public std::streambuf {
 public:
  jsoncbuf(std::streambuf* sbuf)
      : escape_state_(EscapeState::NotEscaped),
        fis_quoted_(false),
        pch_out_max_(rgch_out_ + kCchBufMax -
                     1)  // Leave room in case we need to add two characters for
                         // a not-actually-a-comment sequence
        ,
        rgch_out_(),
        sbuf_input_(sbuf),
        skip_state_(SkipState::NotSkipping) {
    // Intialize the buffer to empty
    setg(rgch_out_, rgch_out_ + 1, rgch_out_ + 1);
  }
  ~jsoncbuf() {}

  int underflow() override {
    traits_type::int_type i = 0;

    // Refill the "get area"
    char* pch_out = rgch_out_;

    while (pch_out < pch_out_max_) {
      char ch;

      i = sbuf_input_->sbumpc();

      // End of input stream
      if (traits_type::eq_int_type(i, traits_type::eof())) break;

      ch = traits_type::to_char_type(i);

      if (escape_state_ == EscapeState::NotEscaped) {
        switch (skip_state_) {
          default:
            break;

          case SkipState::CommentStart:
            if (ch == '*')
              skip_state_ = SkipState::SkippingBlockComment;
            else if (ch == '/')
              skip_state_ = SkipState::SkippingLineComment;
            else {
              skip_state_ = SkipState::NotSkipping;
              *pch_out++ = '/';
            }
            break;

          case SkipState::SkippingBlockComment:
            if (ch == '*')
              skip_state_ = SkipState::SkippingBlockCommentEndStart;
            break;

          case SkipState::SkippingBlockCommentEndStart:
            skip_state_ = (ch == '/') ? SkipState::NotSkipping
                                      : SkipState::SkippingBlockComment;
            break;

          case SkipState::SkippingLineComment:
            if ((ch == '\r') || (ch == '\n'))
              skip_state_ = SkipState::NotSkipping;
            break;
        }  // switch (skip_state_)
      }

      if (skip_state_ == SkipState::NotSkipping) {
        switch (escape_state_) {
          case EscapeState::NotEscaped:
            if (ch == '\\')
              escape_state_ = EscapeState::Escaped;
            else if (ch == '"')
              fis_quoted_ = !fis_quoted_;
            else if (!fis_quoted_) {
              if (ch == '/') skip_state_ = SkipState::CommentStart;
            }
            break;

          case EscapeState::Escaped:
            if (ch == 'u')
              escape_state_ = EscapeState::Hex1;
            else
              escape_state_ = EscapeState::NotEscaped;
            break;

          case EscapeState::Hex1:
            escape_state_ = EscapeState::Hex2;
            break;

          case EscapeState::Hex2:
            escape_state_ = EscapeState::Hex3;
            break;

          case EscapeState::Hex3:
            escape_state_ = EscapeState::NotEscaped;
            break;
        }  // switch (escape_state_)
      }

      if (skip_state_ == SkipState::NotSkipping) *pch_out++ = ch;
    }

    // Set the get area with the next content
    if (pch_out > rgch_out_) {
      setg(rgch_out_, rgch_out_, pch_out);
      return (rgch_out_[0]);
    }

    return (i);
  }

 protected:
  // Escape sequence handling states
  enum class EscapeState {
    NotEscaped,  // Next character is not escaped
    Escaped,     // Next character is escaped
    Hex1,        // First hex sequence character seen
    Hex2,        // Second hex sequence character seen
    Hex3,        // Third hex sequence character seen
  };             // enum class EscapeMode

  // Whether we're skipping input text and the current state
  // of determining whether to start and stop skipping
  enum class SkipState {
    NotSkipping,   // Not skipping comments
    CommentStart,  // Saw "/" of "/*" or "//" that may start a comment

    SkippingBlockComment,  // Skipping until "*/" that ends block comment
    SkippingBlockCommentEndStart,  // Saw "*" or "*/" that end block comment
    SkippingLineComment,           // Skipping until the end of line
  };                               // enum class SkipState

 protected:
  // Maximum output stream buffer size
  static constexpr size_t kCchBufMax = 1024;

 protected:
  bool fis_quoted_;             // If true, we're reading a quoted string
  EscapeState escape_state_;    // Current escaped sequence state
  char* pch_out_max_;           // One past the last entry in rgch_out_
  char rgch_out_[kCchBufMax];   // Output stream buffer we're returning
  std::streambuf* sbuf_input_;  // Input stream buffer we're reading from
  SkipState skip_state_;        // Input skip state
};                              // class jsoncbuf

int LoadJSONCFromFile(const std::string& str_file_path,
                      nlohmann::json* pjson_out) {
  int err_ret = 0;
  std::ifstream ifstream(str_file_path, std::ios::in);

  if (!ifstream.is_open()) err_ret = errno;
  {
    auto streambuf_orig = ifstream.rdbuf();
    jsoncbuf jsonc_buf(streambuf_orig);
    std::istream json_stream(&jsonc_buf);

    try {
      json_stream >> *pjson_out;
    } catch (...) {
      err_ret = EBADMSG;
    }
  }

  return (err_ret);
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
