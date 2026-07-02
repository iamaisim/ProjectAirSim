#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace projectairsim_cpp_tests {
namespace fake_nngi {

void Reset();
void PushServiceResponse(const std::string& response);
void PushTopicMessage(const std::string& message);
std::vector<std::string> SentServiceMessages();
std::vector<std::string> SentTopicMessages();
std::vector<std::string> DialedUrls();

}  // namespace fake_nngi
}  // namespace projectairsim_cpp_tests
