#include "FakeNNGI.h"

#include <NNGI/NNGI.h>

#include <cstring>
#include <deque>
#include <mutex>

namespace projectairsim_cpp_tests {
namespace fake_nngi {
namespace {

std::mutex g_mutex;
std::deque<std::string> g_service_responses;
std::deque<std::string> g_topic_messages;
std::vector<std::string> g_sent_service_messages;
std::vector<std::string> g_sent_topic_messages;
std::vector<std::string> g_dialed_urls;
uint32_t g_next_socket_id = 1;
constexpr uint32_t kServiceSocketId = 1;
constexpr uint32_t kTopicSocketId = 2;

}  // namespace

std::mutex& Mutex() { return g_mutex; }
std::deque<std::string>& ServiceResponses() { return g_service_responses; }
std::deque<std::string>& TopicMessages() { return g_topic_messages; }
std::vector<std::string>& SentServiceMessagesStore() {
  return g_sent_service_messages;
}
std::vector<std::string>& SentTopicMessagesStore() {
  return g_sent_topic_messages;
}
std::vector<std::string>& DialedUrlsStore() { return g_dialed_urls; }
uint32_t& NextSocketId() { return g_next_socket_id; }
uint32_t ServiceSocketId() { return kServiceSocketId; }
uint32_t TopicSocketId() { return kTopicSocketId; }

void Reset() {
  std::lock_guard<std::mutex> lock(g_mutex);
  g_service_responses.clear();
  g_topic_messages.clear();
  g_sent_service_messages.clear();
  g_sent_topic_messages.clear();
  g_dialed_urls.clear();
  g_next_socket_id = 1;
}

void PushServiceResponse(const std::string& response) {
  std::lock_guard<std::mutex> lock(g_mutex);
  g_service_responses.push_back(response);
}

void PushTopicMessage(const std::string& message) {
  std::lock_guard<std::mutex> lock(g_mutex);
  g_topic_messages.push_back(message);
}

std::vector<std::string> SentServiceMessages() {
  std::lock_guard<std::mutex> lock(g_mutex);
  return g_sent_service_messages;
}

std::vector<std::string> SentTopicMessages() {
  std::lock_guard<std::mutex> lock(g_mutex);
  return g_sent_topic_messages;
}

std::vector<std::string> DialedUrls() {
  std::lock_guard<std::mutex> lock(g_mutex);
  return g_dialed_urls;
}

}  // namespace fake_nngi
}  // namespace projectairsim_cpp_tests

namespace nngi {

int nng_close(nng_socket) { return 0; }

int nng_dial(nng_socket, const char* addr, nng_dialer*, int) {
  std::lock_guard<std::mutex> lock(projectairsim_cpp_tests::fake_nngi::Mutex());
  projectairsim_cpp_tests::fake_nngi::DialedUrlsStore().emplace_back(addr);
  return 0;
}

void nng_free(void* buf, std::size_t) { delete[] static_cast<uint8_t*>(buf); }

int nng_pair0_open(nng_socket* socket) {
  std::lock_guard<std::mutex> lock(projectairsim_cpp_tests::fake_nngi::Mutex());
  socket->id = projectairsim_cpp_tests::fake_nngi::NextSocketId()++;
  return 0;
}

int nng_req0_open(nng_socket* socket) {
  std::lock_guard<std::mutex> lock(projectairsim_cpp_tests::fake_nngi::Mutex());
  socket->id = projectairsim_cpp_tests::fake_nngi::NextSocketId()++;
  return 0;
}

int nng_recv(nng_socket socket, void* buf, std::size_t* size, int flags) {
  if ((flags & NNG_FLAG_ALLOC) == 0) return NNG_EINVAL;

  std::lock_guard<std::mutex> lock(projectairsim_cpp_tests::fake_nngi::Mutex());
  if (socket.id == projectairsim_cpp_tests::fake_nngi::TopicSocketId()) {
    if (projectairsim_cpp_tests::fake_nngi::TopicMessages().empty()) {
      return NNG_ETIMEDOUT;
    }

    const std::string response =
        projectairsim_cpp_tests::fake_nngi::TopicMessages().front();
    projectairsim_cpp_tests::fake_nngi::TopicMessages().pop_front();

    auto* bytes = new uint8_t[response.size()];
    std::memcpy(bytes, response.data(), response.size());
    *static_cast<void**>(buf) = bytes;
    *size = response.size();
    return 0;
  }

  if (socket.id != projectairsim_cpp_tests::fake_nngi::ServiceSocketId()) {
    return NNG_ETIMEDOUT;
  }
  if (projectairsim_cpp_tests::fake_nngi::ServiceResponses().empty()) {
    return NNG_ECANCELED;
  }

  const std::string response =
      projectairsim_cpp_tests::fake_nngi::ServiceResponses().front();
  projectairsim_cpp_tests::fake_nngi::ServiceResponses().pop_front();

  auto* bytes = new uint8_t[response.size()];
  std::memcpy(bytes, response.data(), response.size());
  *static_cast<void**>(buf) = bytes;
  *size = response.size();
  return 0;
}

int nng_send(nng_socket socket, void* buf, std::size_t len, int) {
  std::lock_guard<std::mutex> lock(projectairsim_cpp_tests::fake_nngi::Mutex());
  if (socket.id == projectairsim_cpp_tests::fake_nngi::ServiceSocketId()) {
    projectairsim_cpp_tests::fake_nngi::SentServiceMessagesStore().emplace_back(
        static_cast<const char*>(buf), len);
  } else if (socket.id == projectairsim_cpp_tests::fake_nngi::TopicSocketId()) {
    projectairsim_cpp_tests::fake_nngi::SentTopicMessagesStore().emplace_back(
        static_cast<const char*>(buf), len);
  }
  return 0;
}

int nng_socket_set_ms(nng_socket, const char*, nng_duration) { return 0; }

const char* nng_strerror(int error) {
  switch (error) {
    case NNG_ETIMEDOUT:
      return "timed out";
    case NNG_ECLOSED:
      return "closed";
    case NNG_ECANCELED:
      return "canceled";
    case NNG_EINVAL:
      return "invalid";
    default:
      return "fake nng error";
  }
}

const char* nng_version(void) { return "fake-nng"; }

}  // namespace nngi
