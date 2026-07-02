#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <iostream>
#include <string>

namespace asc = microsoft::projectairsim::client;

namespace {

std::string StatusToString(asc::Status status) {
  char buffer[256] = {0};
  asc::GetStatusString(status, buffer);
  return std::string(buffer);
}

}  // namespace

int main(int argc, char** argv) {
  std::string host = "127.0.0.1";
  if (argc > 1) host = argv[1];

  asc::Client client;
  const auto status = client.Connect(host);
  if (status != asc::Status::OK) {
    std::cerr << "[FAIL] Client::Connect(" << host << ") failed: "
              << StatusToString(status) << std::endl;
    std::cerr << "[INFO] Start Unreal/ProjectAirSim simserver before running "
                 "this integration test."
              << std::endl;
    return 1;
  }

  std::cout << "[OK] Client::Connect(" << host
            << ") succeeded against real NNG server" << std::endl;
  client.Disconnect();
  return 0;
}

