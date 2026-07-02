#include <ProjectAirsimClient/ProjectAirsimClient.h>
#include <ProjectAirSimMessage/request_message.hpp>
#include <ProjectAirSimMessage/response_message.hpp>

#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "msgpack.hpp"

#include "FakeNNGI.h"

namespace asc = microsoft::projectairsim::client;
namespace pas = microsoft::projectairsim;
namespace fake_nngi = projectairsim_cpp_tests::fake_nngi;
using json = nlohmann::json;

namespace {

bool Expect(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "[FAIL] " << message << std::endl;
    return false;
  }
  return true;
}

std::string SuccessResponse(int id, const json& result) {
  return pas::ResponseMessage(id, result, asc::kClientAPIVersion).Serialize();
}

std::string ErrorResponse(int id, const std::string& message) {
  return pas::ResponseMessage(id, 1, message, asc::kClientAPIVersion)
      .Serialize();
}

pas::RequestMessage ParseRequest(const std::string& raw) {
  pas::RequestMessage request;
  request.Deserialize(raw);
  return request;
}

void PackTopicInfo(msgpack::packer<msgpack::sbuffer>* packer,
                   const std::string& path) {
  packer->pack_map(4);
  packer->pack(std::string("path"));
  packer->pack(path);
  packer->pack(std::string("type"));
  packer->pack(std::string("topic"));
  packer->pack(std::string("message_type"));
  packer->pack(std::string("msgpack"));
  packer->pack(std::string("frequency"));
  packer->pack(10);
}

std::string TopicListFrame(const std::vector<std::string>& topics) {
  msgpack::sbuffer body;
  msgpack::packer<msgpack::sbuffer> body_packer(body);
  body_packer.pack_array(topics.size());
  for (const auto& topic : topics) PackTopicInfo(&body_packer, topic);

  msgpack::sbuffer frame;
  msgpack::packer<msgpack::sbuffer> frame_packer(frame);
  frame_packer.pack_array(3);
  frame_packer.pack(2);
  frame_packer.pack(std::string("/$topics"));
  frame_packer.pack(std::string(body.data(), body.size()));
  return std::string(frame.data(), frame.size());
}

std::vector<std::string> BasicTopics() {
  return {
      "/Sim/SceneUnit/robots/Drone1/actual_pose",
      "/Sim/SceneUnit/robots/Drone1/collision_info",
      "/Sim/SceneUnit/robots/Drone1/rotor_info",
      "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/scene_camera",
      "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/depth_camera",
      "/Sim/SceneUnit/robots/Drone1/sensors/IMU1/imu_kinematics",
      "/Sim/SceneUnit/robots/Drone1/sensors/GPS/gps",
      "/Sim/SceneUnit/robots/Drone1/sensors/Barometer/barometer",
      "/Sim/SceneUnit/robots/Drone1/sensors/Magnetometer/magnetometer",
      "/Sim/SceneUnit/robots/Drone1/sensors/Airspeed/airspeed",
      "/Sim/SceneUnit/robots/Drone1/sensors/Battery/battery",
      "/Sim/SceneUnit/robots/Drone1/sensors/Radar1/radar_detections",
      "/Sim/SceneUnit/robots/Drone1/sensors/Radar1/radar_tracks",
      "/Sim/SceneUnit/robots/Drone1/sensors/Lidar1/lidar",
  };
}

struct TestEnvironment {
  std::shared_ptr<asc::Client> client;
  std::shared_ptr<asc::World> world;
  std::shared_ptr<asc::Drone> drone;
  std::shared_ptr<asc::Rover> rover;
  std::shared_ptr<asc::EnvActor> env_actor;
  std::shared_ptr<asc::StaticSensorActor> static_sensor_actor;
};

bool CreateWorld(TestEnvironment* env) {
  fake_nngi::Reset();
  fake_nngi::PushTopicMessage(TopicListFrame(BasicTopics()));

  env->client = std::make_shared<asc::Client>();
  env->world = std::make_shared<asc::World>();

  if (!Expect(env->client->Connect("127.0.0.1") == asc::Status::OK,
              "Client::Connect should succeed")) {
    return false;
  }
  if (!Expect(env->world->Initialize(env->client) == asc::Status::OK,
              "World::Initialize should derive scene from topic info")) {
    return false;
  }
  return true;
}

bool CreateDrone(TestEnvironment* env) {
  if (!CreateWorld(env)) return false;

  env->drone = std::make_shared<asc::Drone>();
  if (!Expect(env->drone->Initialize(env->client, env->world, "Drone1") ==
                  asc::Status::OK,
              "Drone::Initialize should derive sensor topics from topic info")) {
    return false;
  }
  return true;
}

bool CreateRover(TestEnvironment* env) {
  if (!CreateWorld(env)) return false;

  env->rover = std::make_shared<asc::Rover>();
  if (!Expect(env->rover->Initialize(env->client, env->world, "Rover1") ==
                  asc::Status::OK,
              "Rover::Initialize should attach to the world topic")) {
    return false;
  }
  return true;
}

bool CreateEnvActor(TestEnvironment* env) {
  if (!CreateWorld(env)) return false;

  env->env_actor = std::make_shared<asc::EnvActor>();
  if (!Expect(env->env_actor->Initialize(env->client, env->world, "Actor1") ==
                  asc::Status::OK,
              "EnvActor::Initialize should attach to the world topic")) {
    return false;
  }
  return true;
}

bool CreateStaticSensorActor(TestEnvironment* env) {
  if (!CreateWorld(env)) return false;

  env->static_sensor_actor = std::make_shared<asc::StaticSensorActor>();
  if (!Expect(env->static_sensor_actor->Initialize(env->client, env->world,
                                                   "Static1") ==
                  asc::Status::OK,
              "StaticSensorActor::Initialize should attach to the world topic")) {
    return false;
  }
  return true;
}

json TransformResultJson() {
  return json{{"timestamp", 123},
              {"frame_id", "world"},
              {"translation", {{"x", 1.0f}, {"y", 2.0f}, {"z", 3.0f}}},
              {"rotation", {{"w", 1.0f}, {"x", 0.0f}, {"y", 0.0f}, {"z", 0.0f}}}};
}

json GeoPositionResultJson() {
  return json{{"latitude", 47.0f}, {"longitude", -122.0f}, {"altitude", 10.0f}};
}

bool ExpectLastRequest(const std::string& method,
                       const std::function<bool(const json&)>& check_params,
                       const std::string& message) {
  auto sent = fake_nngi::SentServiceMessages();
  if (!Expect(!sent.empty(), message + ": should send a service request")) {
    return false;
  }

  const auto request = ParseRequest(sent.back());
  if (!Expect(request.GetMethod() == method,
              message + ": method should be " + method)) {
    return false;
  }
  if (!Expect(check_params(request.GetParams()),
              message + ": params should match")) {
    return false;
  }
  return true;
}

bool ExpectLastRequest(const std::string& method, const std::string& message) {
  return ExpectLastRequest(method, [](const json&) { return true; }, message);
}

bool TestConnectAndRequest() {
  fake_nngi::Reset();
  fake_nngi::PushServiceResponse(SuccessResponse(0, {{"ok", true}}));

  asc::Client client;
  if (!Expect(client.Connect("127.0.0.1") == asc::Status::OK,
              "Client::Connect should succeed with fake NNG")) {
    return false;
  }

  auto dialed_urls = fake_nngi::DialedUrls();
  if (!Expect(dialed_urls.size() == 2, "Connect should dial services and topics")) return false;
  if (!Expect(dialed_urls[0] == "tcp://127.0.0.1:8990",
              "Connect should dial services port 8990")) {
    return false;
  }
  if (!Expect(dialed_urls[1] == "tcp://127.0.0.1:8989",
              "Connect should dial topics port 8989")) {
    return false;
  }

  asc::Message response;
  const auto status =
      client.Request("/Sim/Unit", {{"speed", 3.0}, {"name", "Drone1"}}, &response);
  if (!Expect(status == asc::Status::OK, "Client::Request should receive fake response")) {
    return false;
  }

  auto sent = fake_nngi::SentServiceMessages();
  if (!Expect(sent.size() == 1, "Request should send one service message")) return false;

  const auto request = ParseRequest(sent[0]);
  if (!Expect(request.GetID() == 0, "Request id should be 0")) return false;
  if (!Expect(request.GetMethod() == "/Sim/Unit", "Request method should match")) return false;
  if (!Expect(request.GetParams()["speed"] == 3.0, "Request params should include speed")) {
    return false;
  }
  if (!Expect(request.GetParams()["name"] == "Drone1", "Request params should include name")) {
    return false;
  }

  pas::ResponseMessage response_message;
  response_message.Deserialize(static_cast<std::string>(response));
  if (!Expect(response_message.GetErrorCode() == 0, "Response should be success")) return false;
  if (!Expect(response_message.GetResult()["ok"] == true, "Response result should be preserved")) {
    return false;
  }

  client.Disconnect();
  return true;
}

bool TestClientPublishSendsTopicMessage() {
  fake_nngi::Reset();
  fake_nngi::PushTopicMessage(TopicListFrame(BasicTopics()));

  asc::Client client;
  if (!Expect(client.Connect("127.0.0.1") == asc::Status::OK,
              "Client::Connect should succeed")) {
    return false;
  }

  if (!Expect(client.Publish("/Sim/Topic", {{"value", 42}}) == asc::Status::OK,
              "Client::Publish should succeed")) {
    return false;
  }

  const auto sent_topics = fake_nngi::SentTopicMessages();
  if (!Expect(sent_topics.size() >= 2,
              "Publish should send topic subscribe and message frames")) {
    return false;
  }

  auto object_handle =
      msgpack::unpack(sent_topics.back().data(), sent_topics.back().size());
  msgpack::object frame = object_handle.get();
  std::tuple<int, std::string, std::string> unpacked;
  frame.convert(unpacked);

  if (!Expect(std::get<0>(unpacked) == 2, "Publish frame type should be message")) {
    return false;
  }
  if (!Expect(std::get<1>(unpacked) == "/Sim/Topic",
              "Publish topic should match")) {
    return false;
  }

  client.Disconnect();
  return true;
}

bool TestClientTopicAndLifecycleAPIsUseExpectedMessages() {
  fake_nngi::Reset();
  fake_nngi::PushTopicMessage(TopicListFrame(BasicTopics()));

  asc::Client client;
  if (!Expect(client.Connect("127.0.0.1") == asc::Status::OK,
              "Client::Connect should succeed")) {
    return false;
  }

  const auto topic_info = client.GetTopicInfo();
  if (!Expect(!topic_info.empty(), "Client::GetTopicInfo should return topic paths") ||
      !Expect(topic_info[0] == "/Sim/SceneUnit/robots/Drone1/actual_pose",
              "Client::GetTopicInfo should preserve topic path")) {
    return false;
  }

  if (!Expect(client.Subscribe("/Sim/Topic",
                               [](const std::string&, const std::string&) {}) ==
                  asc::Status::OK,
              "Client::Subscribe should succeed")) {
    return false;
  }
  auto sent_topics = fake_nngi::SentTopicMessages();
  if (!Expect(!sent_topics.empty(), "Client::Subscribe should send a topic frame")) {
    return false;
  }

  fake_nngi::PushServiceResponse(SuccessResponse(0, true));
  if (!Expect(client.Unsubscribe(std::vector<std::string>{"/Sim/Topic"}) ==
                  asc::Status::OK,
              "Client::Unsubscribe vector should succeed")) {
    return false;
  }
  if (!ExpectLastRequest(
          "/Sim/Unsubscribe",
          [](const json& p) {
            return p["topic_paths"].size() == 1 &&
                   p["topic_paths"][0] == "/Sim/Topic";
          },
          "Client::Unsubscribe vector")) {
    return false;
  }

  if (!Expect(client.Subscribe("/Sim/Topic2",
                               [](const std::string&, const std::string&) {}) ==
                  asc::Status::OK,
              "Client::Subscribe should succeed before UnsubscribeAll")) {
    return false;
  }
  fake_nngi::PushServiceResponse(SuccessResponse(1, true));
  if (!Expect(client.UnsubscribeAll() == asc::Status::OK,
              "Client::UnsubscribeAll should succeed")) {
    return false;
  }
  if (!ExpectLastRequest("/Sim/Unsubscribe",
                         [](const json& p) {
                           return p["topic_paths"].size() == 1 &&
                                  p["topic_paths"][0] == "/Sim/Topic2";
                         },
                         "Client::UnsubscribeAll")) {
    return false;
  }

  if (!Expect(client.Subscribe("/Sim/Topic3",
                               [](const std::string&, const std::string&) {}) ==
                  asc::Status::OK,
              "Client::Subscribe should succeed before char unsubscribe")) {
    return false;
  }
  if (!Expect(client.Unsubscribe("/Sim/Topic3") == asc::Status::OK,
              "Client::Unsubscribe char should succeed")) {
    return false;
  }
  if (!Expect(client.UnsubscribeAll() == asc::Status::OK,
              "Client::UnsubscribeAll should be OK after char unsubscribe")) {
    return false;
  }

  bool enabled = false;
  fake_nngi::PushServiceResponse(SuccessResponse(2, true));
  if (!Expect(client.SetInteractiveFeature("weather", true, &enabled) ==
                  asc::Status::OK,
              "Client::SetInteractiveFeature should succeed") ||
      !Expect(enabled, "Client::SetInteractiveFeature should parse bool") ||
      !ExpectLastRequest(
          "/Sim/SetInteractiveFeature",
          [](const json& p) {
            return p["feature_id"] == "weather" && p["enable"] == true;
          },
          "Client::SetInteractiveFeature")) {
    return false;
  }

  client.Disconnect();
  asc::Message response;
  return Expect(client.Request("/Sim/Unit", json::object(), &response) ==
                    asc::Status::NotConnected,
                "Client::Disconnect should close service requests");
}

bool TestRequestBeforeConnectFails() {
  fake_nngi::Reset();

  asc::Client client;
  asc::Message response;
  const auto status = client.Request("/Sim/Unit", json::object(), &response);
  return Expect(status == asc::Status::NotConnected,
                "Request before Connect should return NotConnected");
}

bool TestRejectedResponseReturnsRejectedByServer() {
  TestEnvironment env;
  if (!CreateWorld(&env)) return false;

  fake_nngi::PushServiceResponse(ErrorResponse(0, "unit failure"));
  std::string clock_type;
  const auto status = env.world->GetSimClockType(&clock_type);
  env.client->Disconnect();
  return Expect(status == asc::Status::RejectedByServer,
                "World wrapper server error should return RejectedByServer");
}

bool TestGetBuildCommitHashUsesExpectedServiceMethod() {
  fake_nngi::Reset();
  fake_nngi::PushServiceResponse(SuccessResponse(0, "abc123"));

  asc::Client client;
  if (!Expect(client.Connect("localhost") == asc::Status::OK,
              "Client::Connect should succeed")) {
    return false;
  }

  std::string hash;
  if (!Expect(client.GetBuildCommitHash(&hash) == asc::Status::OK,
              "GetBuildCommitHash should succeed")) {
    return false;
  }
  if (!Expect(hash == "abc123", "GetBuildCommitHash should parse string result")) return false;

  auto sent = fake_nngi::SentServiceMessages();
  if (!Expect(sent.size() == 1, "GetBuildCommitHash should send one request")) return false;
  const auto request = ParseRequest(sent[0]);
  if (!Expect(request.GetMethod() == "/Sim/GetBuildCommitHash",
              "GetBuildCommitHash should use expected service method")) {
    return false;
  }

  client.Disconnect();
  return true;
}

bool TestWorldAPIsUseExpectedServiceMethods() {
  TestEnvironment env;
  if (!CreateWorld(&env)) return false;

  int id = 0;
  auto push = [&](const json& result) {
    fake_nngi::PushServiceResponse(SuccessResponse(id++, result));
  };

  std::string str_out;
  std::int64_t time_out = 0;
  bool bool_out = false;
  float float_out = 0.0f;
  asc::Vector3 vector_out;
  asc::VecStr names_out;
  std::vector<float> scale_out;
  json json_out;

  push("steppable");
  if (!Expect(env.world->GetSimClockType(&str_out) == asc::Status::OK,
              "World::GetSimClockType should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/GetSimClockType",
                         "World::GetSimClockType")) {
    return false;
  }

  push(1234);
  if (!Expect(env.world->GetSimTime(&time_out) == asc::Status::OK,
              "World::GetSimTime should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/GetSimTime", "World::GetSimTime")) {
    return false;
  }

  push("paused");
  if (!Expect(env.world->Pause(&str_out) == asc::Status::OK,
              "World::Pause should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/Pause",
                         [](const json& p) { return p["do_pause"] == true; },
                         "World::Pause")) {
    return false;
  }

  push("running");
  if (!Expect(env.world->Resume(&str_out) == asc::Status::OK,
              "World::Resume should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/Pause",
                         [](const json& p) { return p["do_pause"] == false; },
                         "World::Resume")) {
    return false;
  }

  push(true);
  if (!Expect(env.world->IsPaused(&bool_out) == asc::Status::OK,
              "World::IsPaused should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/IsPaused", "World::IsPaused")) {
    return false;
  }

  push(2000);
  if (!Expect(env.world->ContinueForSimTime(100, true, &time_out) ==
                  asc::Status::OK,
              "World::ContinueForSimTime should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/ContinueForSimTime",
                         [](const json& p) {
                           return p["delta_time"] == 100 &&
                                  p["wait_until_complete"] == true;
                         },
                         "World::ContinueForSimTime")) {
    return false;
  }

  push(3000);
  if (!Expect(env.world->ContinueUntilSimTime(3000, false, &time_out) ==
                  asc::Status::OK,
              "World::ContinueUntilSimTime should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/ContinueUntilSimTime",
                         [](const json& p) {
                           return p["target_time"] == 3000 &&
                                  p["wait_until_complete"] == false;
                         },
                         "World::ContinueUntilSimTime")) {
    return false;
  }

  push(4000);
  if (!Expect(env.world->ContinueForNSteps(4, true, &time_out) ==
                  asc::Status::OK,
              "World::ContinueForNSteps should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/ContinueForNSteps",
                         [](const json& p) {
                           return p["n_steps"] == 4 &&
                                  p["wait_until_complete"] == true;
                         },
                         "World::ContinueForNSteps")) {
    return false;
  }

  push(5000);
  if (!Expect(env.world->ContinueForSingleStep(false, &time_out) ==
                  asc::Status::OK,
              "World::ContinueForSingleStep should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/ContinueForSingleStep",
                         [](const json& p) {
                           return p["wait_until_complete"] == false;
                         },
                         "World::ContinueForSingleStep")) {
    return false;
  }

  struct BoolCase {
    std::string name;
    std::function<asc::Status()> call;
    std::string method;
    std::function<bool(const json&)> params;
  };

  const std::vector<BoolCase> bool_cases = {
      {"SetSunlightIntensity",
       [&]() { return env.world->SetSunlightIntensity(10.0f, &bool_out); },
       "/Sim/SceneUnit/SetSunLightIntensity",
       [](const json& p) { return p["intensity"] == 10.0f; }},
      {"SetCloudShadowStrength",
       [&]() { return env.world->SetCloudShadowStrength(0.5f, &bool_out); },
       "/Sim/SceneUnit/SetCloudShadowStrength",
       [](const json& p) { return p["strength"] == 0.5f; }},
      {"SetWindVelocity",
       [&]() { return env.world->SetWindVelocity(1.0f, 2.0f, 3.0f, &bool_out); },
       "/Sim/SceneUnit/SetWindVelocity",
       [](const json& p) {
         return p["v_x"] == 1.0f && p["v_y"] == 2.0f && p["v_z"] == 3.0f;
       }},
      {"SetObjectPose",
       [&]() {
         return env.world->SetObjectPose("Cube", {{"pose", true}}, true,
                                         &bool_out);
       },
       "/Sim/SceneUnit/SetObjectPose",
       [](const json& p) {
         return p["object_name"] == "Cube" && p["teleport"] == true;
       }},
      {"SetObjectScale",
       [&]() {
         return env.world->SetObjectScale("Cube", {1.0f, 2.0f, 3.0f},
                                          &bool_out);
       },
       "/Sim/SceneUnit/SetObjectScale",
       [](const json& p) {
         return p["object_name"] == "Cube" && p["scale"].size() == 3;
       }},
      {"DestroyObject",
       [&]() { return env.world->DestroyObject("Cube", &bool_out); },
       "/Sim/SceneUnit/DestroyObject",
       [](const json& p) { return p["object_name"] == "Cube"; }},
      {"DestroyAllSpawnedObjects",
       [&]() { return env.world->DestroyAllSpawnedObjects(&bool_out); },
       "/Sim/SceneUnit/DestroyAllSpawnedObjects",
       [](const json&) { return true; }},
      {"EnableWeatherVisualEffects",
       [&]() { return env.world->EnableWeatherVisualEffects(&bool_out); },
       "/Sim/SceneUnit/SimSetWeatherVisualEffectsStatus",
       [](const json& p) { return p["status"] == true; }},
      {"DisableWeatherVisualEffects",
       [&]() { return env.world->DisableWeatherVisualEffects(&bool_out); },
       "/Sim/SceneUnit/SimSetWeatherVisualEffectsStatus",
       [](const json& p) { return p["status"] == false; }},
      {"ResetWeatherEffects",
       [&]() { return env.world->ResetWeatherEffects(&bool_out); },
       "/Sim/SceneUnit/ResetWeatherEffects",
       [](const json&) { return true; }},
      {"SetWeatherVisualEffectsParam",
       [&]() {
         return env.world->SetWeatherVisualEffectsParam(2, 0.75f, &bool_out);
       },
       "/Sim/SceneUnit/SetWeatherVisualEffectsParameter",
       [](const json& p) { return p["param"] == 2 && p["value"] == 0.75f; }},
      {"SetObjectMaterial",
       [&]() {
         return env.world->SetObjectMaterial("Cube", "/Game/Mat", &bool_out);
       },
       "/Sim/SceneUnit/SetObjectMaterial",
       [](const json& p) {
         return p["object_name"] == "Cube" &&
                p["material_asset_path"] == "/Game/Mat";
       }},
      {"SetObjectTextureFromUrl",
       [&]() {
         return env.world->SetObjectTextureFromUrl("Cube", "http://tex",
                                                   &bool_out);
       },
       "/Sim/SceneUnit/SetObjectTextureFromUrl",
       [](const json& p) { return p["url"] == "http://tex"; }},
      {"SetObjectTextureFromFile",
       [&]() {
         return env.world->SetObjectTextureFromFile("Cube", "/tmp/t.png",
                                                    &bool_out);
       },
       "/Sim/SceneUnit/SetObjectTextureFromFile",
       [](const json& p) { return p["texture_file_path"] == "/tmp/t.png"; }},
      {"SetObjectTextureFromPackagedAsset",
       [&]() {
         return env.world->SetObjectTextureFromPackagedAsset(
             "Cube", "/Game/Tex", &bool_out);
       },
       "/Sim/SceneUnit/SetObjectTextureFromPackagedAsset",
       [](const json& p) { return p["texture_asset_path"] == "/Game/Tex"; }},
      {"SwapObjectTexture",
       [&]() { return env.world->SwapObjectTexture("tag", 3, &bool_out); },
       "/Sim/SceneUnit/SwapObjectTexture",
       [](const json& p) { return p["tag"] == "tag" && p["tex_id"] == 3; }},
      {"SetLightObjectIntensity",
       [&]() {
         return env.world->SetLightObjectIntensity("Light", 50.0f, &bool_out);
       },
       "/Sim/SceneUnit/SetLightObjectIntensity",
       [](const json& p) { return p["new_intensity"] == 50.0f; }},
      {"SetLightObjectColor",
       [&]() {
         return env.world->SetLightObjectColor("Light", {1.0f, 0.5f, 0.0f},
                                               &bool_out);
       },
       "/Sim/SceneUnit/SetLightObjectColor",
       [](const json& p) { return p["color_rgb"].size() == 3; }},
      {"SetLightObjectRadius",
       [&]() { return env.world->SetLightObjectRadius("Light", 2.0f, &bool_out); },
       "/Sim/SceneUnit/SetLightObjectRadius",
       [](const json& p) { return p["new_radius"] == 2.0f; }},
      {"SetTimeOfDay",
       [&]() {
         return env.world->SetTimeOfDay(true, "2026-01-01 12:00:00", false,
                                        1.0f, 60.0f, true, &bool_out);
       },
       "/Sim/SceneUnit/SetTimeOfDay",
       [](const json& p) {
         return p["status"] == true && p["move_sun"] == true &&
                p["datetime"] == "2026-01-01 12:00:00";
       }},
      {"SetSunPositionFromDateTime",
       [&]() {
         return env.world->SetSunPositionFromDateTime("2026-01-01", false,
                                                      &bool_out);
       },
       "/Sim/SceneUnit/SetSunPositionFromDateTime",
       [](const json& p) { return p["date_time"] == "2026-01-01"; }},
      {"SwitchStreamingView",
       [&]() { return env.world->SwitchStreamingView(&bool_out); },
       "/Sim/SceneUnit/SwitchStreamingView",
       [](const json&) { return true; }},
      {"SetSegmentationIDByName",
       [&]() {
         return env.world->SetSegmentationIDByName("Cube", 7, false, true,
                                                   &bool_out);
       },
       "/Sim/SceneUnit/SetSegmentationIDByName",
       [](const json& p) {
         return p["mesh_name"] == "Cube" && p["segmentation_id"] == 7 &&
                p["use_owner_name"] == true;
       }},
      {"SetTraceLine",
       [&]() {
         asc::ColorRGBA color{1.0f, 0.0f, 0.0f, 1.0f};
         return env.world->SetTraceLine(color, 2.0f, &bool_out);
       },
       "/Sim/SceneUnit/SetTraceLine",
       [](const json& p) { return p["thickness"] == 2.0f; }},
      {"ToggleTrace",
       [&]() { return env.world->ToggleTrace(&bool_out); },
       "/Sim/SceneUnit/ToggleTrace",
       [](const json&) { return true; }},
  };

  for (const auto& c : bool_cases) {
    push(true);
    if (!Expect(c.call() == asc::Status::OK, c.name + " should succeed") ||
        !ExpectLastRequest(c.method, c.params, c.name)) {
      return false;
    }
  }

  struct JsonCase {
    std::string name;
    std::function<asc::Status()> call;
    std::string method;
    json result;
    std::function<bool(const json&)> params;
  };

  const std::vector<JsonCase> json_cases = {
      {"GetSunlightIntensity",
       [&]() { return env.world->GetSunlightIntensity(&float_out); },
       "/Sim/SceneUnit/GetSunLightIntensity", 42.0f,
       [](const json&) { return true; }},
      {"GetCloudShadowStrength",
       [&]() { return env.world->GetCloudShadowStrength(&float_out); },
       "/Sim/SceneUnit/GetCloudShadowStrength", 0.5f,
       [](const json&) { return true; }},
      {"GetWindVelocity",
       [&]() { return env.world->GetWindVelocity(&vector_out); },
       "/Sim/SceneUnit/GetWindVelocity", json::array({1.0f, 2.0f, 3.0f}),
       [](const json&) { return true; }},
      {"ListActors",
       [&]() { return env.world->ListActors(&names_out); },
       "/Sim/SceneUnit/ListActors", json::array({"Drone1"}),
       [](const json&) { return true; }},
      {"ListObjects",
       [&]() { return env.world->ListObjects(".*", &names_out); },
       "/Sim/SceneUnit/ListObjects", json::array({"Cube"}),
       [](const json& p) { return p["name"] == ".*"; }},
      {"ListAssets",
       [&]() { return env.world->ListAssets(".*", &names_out); },
       "/Sim/SceneUnit/ListAssets", json::array({"Asset"}),
       [](const json& p) { return p["name"] == ".*"; }},
      {"GetObjectPose",
       [&]() { return env.world->GetObjectPose("Cube", &json_out); },
       "/Sim/SceneUnit/GetObjectPose",
       json{{"translation", {{"x", 0}, {"y", 0}, {"z", 0}}},
            {"rotation", {{"w", 1}, {"x", 0}, {"y", 0}, {"z", 0}}}},
       [](const json& p) { return p["object_name"] == "Cube"; }},
      {"GetObjectPoses",
       [&]() { return env.world->GetObjectPoses({"Cube"}, &json_out); },
       "/Sim/SceneUnit/GetObjectPoses", json::array(),
       [](const json& p) { return p["object_names"].size() == 1; }},
      {"GetObjectScale",
       [&]() { return env.world->GetObjectScale("Cube", &scale_out); },
       "/Sim/SceneUnit/GetObjectScale", json::array({1.0f, 1.0f, 1.0f}),
       [](const json& p) { return p["object_name"] == "Cube"; }},
      {"SpawnObject",
       [&]() {
         return env.world->SpawnObject("Cube", "/Game/Cube", {{"pose", true}},
                                       {1.0f, 1.0f, 1.0f}, false, &str_out);
       },
       "/Sim/SceneUnit/SpawnObject", "Cube_1",
       [](const json& p) {
         return p["object_name"] == "Cube" && p["asset_path"] == "/Game/Cube";
       }},
      {"SpawnObjectFromFile",
       [&]() {
         return env.world->SpawnObjectFromFile(
             "Mesh", "gltf", {1, 2, 3}, true, {{"pose", true}},
             {1.0f, 1.0f, 1.0f}, false, &str_out);
       },
       "/Sim/SceneUnit/spawnObjectFromFile", "Mesh_1",
       [](const json& p) {
         return p["object_name"] == "Mesh" && p["file_format"] == "gltf" &&
                p["is_binary"] == true;
       }},
      {"SpawnObjectAtGeo",
       [&]() {
         return env.world->SpawnObjectAtGeo(
             "Geo", "/Game/Cube", 1.0f, 2.0f, 3.0f,
             {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, false, &str_out);
       },
       "/Sim/SceneUnit/spawnObjectAtGeo", "Geo_1",
       [](const json& p) {
         return p["latitude"] == 1.0f && p["longitude"] == 2.0f;
       }},
      {"SpawnObjectFromFileAtGeo",
       [&]() {
         return env.world->SpawnObjectFromFileAtGeo(
             "GeoFile", "gltf", {1}, true, 1.0f, 2.0f, 3.0f,
             {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, false, &str_out);
       },
       "/Sim/SceneUnit/spawnObjectFromFileAtGeo", "GeoFile_1",
       [](const json& p) {
         return p["object_name"] == "GeoFile" && p["byte_array"].size() == 1;
       }},
      {"GetWeatherVisualEffectsParam",
       [&]() { return env.world->GetWeatherVisualEffectsParam(&json_out); },
       "/Sim/SceneUnit/GetWeatherVisualEffectsParameter", json{{"0", 0.1f}},
       [](const json&) { return true; }},
      {"GetTimeOfDay",
       [&]() { return env.world->GetTimeOfDay(&json_out); },
       "/Sim/SceneUnit/GetTimeOfDay", json{{"enabled", true}},
       [](const json&) { return true; }},
      {"GetSegmentationIDByName",
       [&]() {
         int seg_id = 0;
         return env.world->GetSegmentationIDByName("Cube", true, &seg_id);
       },
       "/Sim/SceneUnit/GetSegmentationIDByName", 7,
       [](const json& p) {
         return p["mesh_name"] == "Cube" && p["use_owner_name"] == true;
       }},
      {"GetSegmentationIDMap",
       [&]() { return env.world->GetSegmentationIDMap(&json_out); },
       "/Sim/SceneUnit/GetSegmentationIDMap", json{{"Cube", 7}},
       [](const json&) { return true; }},
      {"Get3DBoundingBox",
       [&]() { return env.world->Get3DBoundingBox("Cube", 0, &json_out); },
       "/Sim/SceneUnit/Get3DBoundingBox", json{{"name", "Cube"}},
       [](const json& p) {
         return p["object_name"] == "Cube" && p["box_alignment"] == 0;
       }},
      {"GetSurfaceElevationAtPoint",
       [&]() { return env.world->GetSurfaceElevationAtPoint(1.0f, 2.0f, &float_out); },
       "/Sim/SceneUnit/GetSurfaceElevationAtPoint", 12.5f,
       [](const json& p) { return p["x"] == 1.0f && p["y"] == 2.0f; }},
  };

  for (const auto& c : json_cases) {
    push(c.result);
    if (!Expect(c.call() == asc::Status::OK, c.name + " should succeed") ||
        !ExpectLastRequest(c.method, c.params, c.name)) {
      return false;
    }
  }

  asc::ColorRGBA color{1.0f, 1.0f, 1.0f, 1.0f};
  asc::VecVector3 points = {{0.0f, 0.0f, 0.0f}};
  asc::VecVector3 points_end = {{1.0f, 0.0f, 0.0f}};
  asc::VecPose poses = {asc::Pose()};

  const std::vector<BoolCase> debug_cases = {
      {"FlushPersistentMarkers",
       [&]() { return env.world->FlushPersistentMarkers(); },
       "/Sim/SceneUnit/debugFlushPersistentMarkers",
       [](const json&) { return true; }},
      {"PlotDebugPoints",
       [&]() { return env.world->PlotDebugPoints(points, color, 1.0f, 2.0f, false); },
       "/Sim/SceneUnit/debugPlotPoints",
       [](const json& p) { return p["size"] == 1.0f; }},
      {"PlotDebugSolidLine",
       [&]() { return env.world->PlotDebugSolidLine(points, color, 1.0f, 2.0f, false); },
       "/Sim/SceneUnit/debugPlotSolidLine",
       [](const json& p) { return p["thickness"] == 1.0f; }},
      {"PlotDebugDashedLine",
       [&]() {
         asc::VecVector3 even_points = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};
         return env.world->PlotDebugDashedLine(even_points, color, 1.0f, 2.0f,
                                               false);
       },
       "/Sim/SceneUnit/debugPlotDashedLine",
       [](const json& p) { return p["points"].size() == 2; }},
      {"PlotDebugArrows",
       [&]() {
         return env.world->PlotDebugArrows(points, points_end, color, 1.0f, 0.5f,
                                           2.0f, false);
       },
       "/Sim/SceneUnit/debugPlotArrows",
       [](const json& p) { return p["arrow_size"] == 0.5f; }},
      {"PlotDebugStrings",
       [&]() { return env.world->PlotDebugStrings({"hi"}, points, 1.0f, color, 2.0f); },
       "/Sim/SceneUnit/debugPlotStrings",
       [](const json& p) { return p["strings"].size() == 1; }},
      {"PlotDebugTransforms",
       [&]() { return env.world->PlotDebugTransforms(poses, 1.0f, 0.5f, 2.0f, false); },
       "/Sim/SceneUnit/debugPlotTransforms",
       [](const json& p) { return p["poses"].size() == 1; }},
      {"PlotDebugTransformsWithNames",
       [&]() {
         return env.world->PlotDebugTransformsWithNames(poses, {"pose"}, 1.0f,
                                                        0.5f, 1.0f, color, 2.0f);
       },
       "/Sim/SceneUnit/debugPlotTransformsWithNames",
       [](const json& p) { return p["names"].size() == 1; }},
      {"ImportGeoTrajectory",
       [&]() {
         return env.world->ImportGeoTrajectory("geo", {0.0f}, {1.0f}, {2.0f},
                                               {3.0f});
       },
       "/Sim/SceneUnit/ImportGeoTrajectory",
       [](const json& p) { return p["traj_name"] == "geo"; }},
  };

  for (const auto& c : debug_cases) {
    push(true);
    if (!Expect(c.call() == asc::Status::OK, c.name + " should succeed") ||
        !ExpectLastRequest(c.method, c.params, c.name)) {
      return false;
    }
  }

  push(true);
  if (!Expect(env.world->ImportNEDTrajectory("ned", {0.0f}, {1.0f}, {2.0f},
                                             {3.0f}) == asc::Status::OK,
              "World::ImportNEDTrajectory should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/ImportNEDTrajectory",
                         [](const json& p) { return p["traj_name"] == "ned"; },
                         "World::ImportNEDTrajectory")) {
    return false;
  }

  push(json::array({true, false}));
  asc::BoolArray voxel_grid;
  if (!Expect(env.world->CreateVoxelGrid(asc::Pose(), 2, 2, 2, 1.0f,
                                         &voxel_grid, {"Drone1"}, false) ==
                  asc::Status::OK,
              "World::CreateVoxelGrid should succeed") ||
      !Expect(voxel_grid.Cf() == 2, "World::CreateVoxelGrid should parse bool array") ||
      !ExpectLastRequest("/Sim/SceneUnit/createVoxelGrid",
                         [](const json& p) {
                           return p["x_size"] == 2 && p["y_size"] == 2 &&
                                  p["z_size"] == 2 && p["res"] == 1.0f &&
                                  p["actors_to_ignore"].size() == 1;
                         },
                         "World::CreateVoxelGrid")) {
    return false;
  }

  env.client->Disconnect();
  return true;
}

bool TestDroneAPIsUseExpectedServiceMethods() {
  TestEnvironment env;
  if (!CreateDrone(&env)) return false;

  int id = 0;
  auto push = [&](const json& result) {
    fake_nngi::PushServiceResponse(SuccessResponse(id++, result));
  };

  bool bool_out = false;
  float float_out = 0.0f;
  json json_out;
  asc::ReadyState ready_state;
  asc::LandedState landed_state = asc::LandedState::Flying;
  asc::Transform transform_out;
  asc::GeoPosition geo_out;

  struct DroneCase {
    std::string name;
    std::function<asc::Status()> call;
    std::string method;
    std::function<bool(const json&)> params;
  };

  const std::vector<DroneCase> cases = {
      {"EnableAPIControl",
       [&]() { return env.drone->EnableAPIControl(&bool_out); },
       "/Sim/SceneUnit/robots/Drone1/EnableApiControl",
       [](const json&) { return true; }},
      {"DisableAPIControl",
       [&]() { return env.drone->DisableAPIControl(&bool_out); },
       "/Sim/SceneUnit/robots/Drone1/DisableApiControl",
       [](const json&) { return true; }},
      {"IsAPIControlEnabled",
       [&]() { return env.drone->IsAPIControlEnabled(&bool_out); },
       "/Sim/SceneUnit/robots/Drone1/IsApiControlEnabled",
       [](const json&) { return true; }},
      {"Arm",
       [&]() { return env.drone->Arm(&bool_out); },
       "/Sim/SceneUnit/robots/Drone1/Arm",
       [](const json&) { return true; }},
      {"Disarm",
       [&]() { return env.drone->Disarm(&bool_out); },
       "/Sim/SceneUnit/robots/Drone1/Disarm",
       [](const json&) { return true; }},
      {"CanArm",
       [&]() { return env.drone->CanArm(&bool_out); },
       "/Sim/SceneUnit/robots/Drone1/CanArm",
       [](const json&) { return true; }},
      {"SetGroundTruthKinematics",
       [&]() {
         return env.drone->SetGroundTruthKinematics({{"linear", true}},
                                                    &bool_out);
       },
       "/Sim/SceneUnit/robots/Drone1/SetGroundTruthKinematics",
       [](const json& p) { return p["kinematics"]["linear"] == true; }},
      {"SetCameraPose",
       [&]() {
         return env.drone->SetCameraPose("Camera1", {{"pose", true}}, true,
                                         &bool_out);
       },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/SetPose",
       [](const json& p) {
         return p["pose"]["pose"] == true && p["wait_for_pose_update"] == true;
       }},
      {"ResetCameraPose",
       [&]() { return env.drone->ResetCameraPose("Camera1", false, &bool_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/ResetCameraPose",
       [](const json& p) { return p["wait_for_pose_update"] == false; }},
      {"SetFocalLength",
       [&]() { return env.drone->SetFocalLength("Camera1", 0, 35.0f, &bool_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/SetFocalLength",
       [](const json& p) {
         return p["image_type_id"] == 0 && p["focal_length"] == 35.0f;
       }},
      {"SetDepthOfFieldTransitionThreshold",
       [&]() {
         return env.drone->SetDepthOfFieldTransitionThreshold("Camera1", 0, 4.0f,
                                                              &bool_out);
       },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/SetDepthOfFieldTransitionRegion",
       [](const json& p) { return p["transition_threshold"] == 4.0f; }},
      {"SetDepthOfFieldFocalRegion",
       [&]() {
         return env.drone->SetDepthOfFieldFocalRegion("Camera1", 0, 10.0f,
                                                      &bool_out);
       },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/SetDepthOfFieldFocalRegion",
       [](const json& p) { return p["focal_length"] == 10.0f; }},
      {"SetChromaticAberrationIntensity",
       [&]() {
         return env.drone->SetChromaticAberrationIntensity("Camera1", 0, 0.25f,
                                                           &bool_out);
       },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/SetChromaticAberrationIntensity",
       [](const json& p) { return p["intensity"] == 0.25f; }},
      {"SetFieldOfView",
       [&]() { return env.drone->SetFieldOfView("Camera1", 0, 90.0f, &bool_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/SetFieldOfView",
       [](const json& p) { return p["field_of_view"] == 90.0f; }},
      {"SetControlSignals",
       [&]() {
         return env.drone->SetControlSignals({{"0", 0.5f}}, &bool_out);
       },
       "/Sim/SceneUnit/robots/Drone1/SetControlSignals",
       [](const json& p) { return p["control_signal_map"]["0"] == 0.5f; }},
      {"UpdateActuatorFaultState",
       [&]() {
         return env.drone->UpdateActuatorFaultState("0", true, &bool_out);
       },
       "/Sim/SceneUnit/robots/Drone1/actuators/0/ToggleFault",
       [](const json& p) { return p["enable"] == true; }},
      {"SetExternalForce",
       [&]() { return env.drone->SetExternalForce({1.0f, 2.0f, 3.0f}, &bool_out); },
       "/Sim/SceneUnit/robots/Drone1/SetExternalForce",
       [](const json& p) { return p["ext_force"].size() == 3; }},
      {"SetBatteryRemaining",
       [&]() { return env.drone->SetBatteryRemaining(0.5f, &bool_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Battery/SetBatteryRemaining",
       [](const json& p) { return p["desired_battery_remaining"] == 0.5f; }},
      {"SetBatteryDrainRate",
       [&]() { return env.drone->SetBatteryDrainRate(0.25f, &bool_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Battery/SetBatteryDrainRate",
       [](const json& p) { return p["desired_drain_rate"] == 0.25f; }},
      {"SetBatteryHealthStatus",
       [&]() { return env.drone->SetBatteryHealthStatus(true, &bool_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Battery/SetBatteryHealthStatus",
       [](const json& p) { return p["battery_health_indicator"] == true; }},
      {"CameraLookAtObject",
       [&]() {
         return env.drone->CameraLookAtObject("Camera1", "Cube", true,
                                              &bool_out);
       },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/LookAtObject",
       [](const json& p) {
         return p["object_name"] == "Cube" && p["wait_for_pose_update"] == true;
       }},
      {"CameraDrawFrustum",
       [&]() {
         return env.drone->CameraDrawFrustum("Camera1", true,
                                             asc::Drone::ImageType::Scene,
                                             &bool_out);
       },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/DrawFrustum",
       [](const json& p) { return p["to_enable"] == true && p["image_type"] == 0; }},
  };

  for (const auto& c : cases) {
    push(true);
    if (!Expect(c.call() == asc::Status::OK, c.name + " should succeed") ||
        !ExpectLastRequest(c.method, c.params, c.name)) {
      return false;
    }
  }

  struct AsyncCase {
    std::string name;
    std::function<asc::AsyncResult()> call;
    std::string method;
    std::function<bool(const json&)> params;
  };

  const std::vector<AsyncCase> async_cases = {
      {"TakeoffAsync",
       [&]() { return env.drone->TakeoffAsync(5.0f); },
       "/Sim/SceneUnit/robots/Drone1/Takeoff",
       [](const json& p) { return p["timeout_sec"] == 5.0f; }},
      {"LandAsync",
       [&]() { return env.drone->LandAsync(6.0f); },
       "/Sim/SceneUnit/robots/Drone1/Land",
       [](const json& p) { return p["timeout_sec"] == 6.0f; }},
      {"GoHomeAsync",
       [&]() { return env.drone->GoHomeAsync(7.0f, 1.5f); },
       "/Sim/SceneUnit/robots/Drone1/GoHome",
       [](const json& p) {
         return p["timeout_sec"] == 7.0f && p["velocity"] == 1.5f;
       }},
      {"HoverAsync",
       [&]() { return env.drone->HoverAsync(); },
       "/Sim/SceneUnit/robots/Drone1/Hover",
       [](const json&) { return true; }},
      {"RequestControlAsync",
       [&]() { return env.drone->RequestControlAsync(); },
       "/Sim/SceneUnit/robots/Drone1/RequestControl",
       [](const json&) { return true; }},
      {"SetMissionModeAsync",
       [&]() { return env.drone->SetMissionModeAsync(); },
       "/Sim/SceneUnit/robots/Drone1/SetMissionMode",
       [](const json&) { return true; }},
      {"SetVTOLModeAsync",
       [&]() { return env.drone->SetVTOLModeAsync(asc::Drone::VTOLMode::FixedWing); },
       "/Sim/SceneUnit/robots/Drone1/SetVTOLMode",
       [](const json& p) { return p["vtol_mode"] == 1; }},
      {"MoveByHeadingAsync",
       [&]() {
         return env.drone->MoveByHeadingAsync(1.0f, 2.0f, -0.5f, 3.0f, 0.1f,
                                              0.2f, 4.0f);
       },
       "/Sim/SceneUnit/robots/Drone1/MoveByHeading",
       [](const json& p) {
         return p["heading"] == 1.0f && p["speed"] == 2.0f &&
                p["vz"] == -0.5f;
       }},
      {"MoveByVelocityAsync",
       [&]() {
         return env.drone->MoveByVelocityAsync(1.0f, 2.0f, 3.0f, 4.0f);
       },
       "/Sim/SceneUnit/robots/Drone1/MoveByVelocity",
       [](const json& p) { return p["vx"] == 1.0f && p["duration"] == 4.0f; }},
      {"MoveByVelocityZAsync",
       [&]() {
         return env.drone->MoveByVelocityZAsync(1.0f, 2.0f, -3.0f, 4.0f);
       },
       "/Sim/SceneUnit/robots/Drone1/MoveByVelocityZ",
       [](const json& p) { return p["z"] == -3.0f && p["duration"] == 4.0f; }},
      {"MoveByVelocityBodyFrameAsync",
       [&]() {
         return env.drone->MoveByVelocityBodyFrameAsync(1.0f, 2.0f, 3.0f,
                                                        4.0f);
       },
       "/Sim/SceneUnit/robots/Drone1/MoveByVelocityBodyFrame",
       [](const json& p) { return p["vx"] == 1.0f && p["vy"] == 2.0f; }},
      {"MoveByVelocityBodyFrameZAsync",
       [&]() {
         return env.drone->MoveByVelocityBodyFrameZAsync(1.0f, 2.0f, -3.0f,
                                                         4.0f);
       },
       "/Sim/SceneUnit/robots/Drone1/MoveByVelocityBodyFrameZ",
       [](const json& p) { return p["z"] == -3.0f && p["duration"] == 4.0f; }},
      {"MoveToPositionAsync",
       [&]() {
         return env.drone->MoveToPositionAsync(1.0f, 2.0f, -3.0f, 4.0f,
                                               5.0f);
       },
       "/Sim/SceneUnit/robots/Drone1/MoveToPosition",
       [](const json& p) { return p["x"] == 1.0f && p["z"] == -3.0f; }},
      {"MoveOnPathAsync",
       [&]() {
         return env.drone->MoveOnPathAsync({{1.0f, 2.0f, -3.0f}}, 4.0f, 5.0f);
       },
       "/Sim/SceneUnit/robots/Drone1/MoveOnPath",
       [](const json& p) { return p["path"].size() == 1 && p["velocity"] == 4.0f; }},
      {"RotateToYawAsync",
       [&]() { return env.drone->RotateToYawAsync(90.0f, 5.0f, 0.1f, 0.2f); },
       "/Sim/SceneUnit/robots/Drone1/RotateToYaw",
       [](const json& p) { return p["yaw"] == 90.0f && p["timeout_sec"] == 5.0f; }},
      {"RotateByYawRateAsync",
       [&]() { return env.drone->RotateByYawRateAsync(0.5f, 2.0f); },
       "/Sim/SceneUnit/robots/Drone1/RotateByYawRate",
       [](const json& p) { return p["yaw_rate"] == 0.5f && p["duration"] == 2.0f; }},
  };

  for (const auto& c : async_cases) {
    push(true);
    auto result = c.call();
    if (!Expect(result.Wait() == asc::Status::OK,
                c.name + " should complete successfully") ||
        !ExpectLastRequest(c.method, c.params, c.name)) {
      return false;
    }
  }

  push({{"imu", true}});
  if (!Expect(env.drone->GetIMUData("IMU1", &json_out) == asc::Status::OK,
              "Drone::GetIMUData should succeed") ||
      !ExpectLastRequest(
          "/Sim/SceneUnit/robots/Drone1/sensors/IMU1/imu_kinematics",
          "Drone::GetIMUData")) {
    return false;
  }

  struct JsonDroneCase {
    std::string name;
    std::function<asc::Status()> call;
    std::string method;
    json result;
    std::function<bool(const json&)> params;
  };

  const std::vector<JsonDroneCase> json_cases = {
      {"GetGPSData",
       [&]() { return env.drone->GetGPSData("GPS", &json_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/GPS/gps", json{{"gps", true}},
       [](const json&) { return true; }},
      {"GetAirspeedData",
       [&]() { return env.drone->GetAirspeedData("Airspeed", &json_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Airspeed/airspeed",
       json{{"airspeed", true}}, [](const json&) { return true; }},
      {"GetBatteryState",
       [&]() { return env.drone->GetBatteryState(&json_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Battery/GetBatteryStatus",
       json{{"battery", true}}, [](const json&) { return true; }},
      {"GetBatteryDrainRate",
       [&]() { return env.drone->GetBatteryDrainRate(&float_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Battery/GetBatteryDrainRate", 0.1f,
       [](const json&) { return true; }},
      {"GetBarometerData",
       [&]() { return env.drone->GetBarometerData("Barometer", &json_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Barometer/barometer",
       json{{"barometer", true}}, [](const json&) { return true; }},
      {"GetMagnetometerData",
       [&]() { return env.drone->GetMagnetometerData("Magnetometer", &json_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Magnetometer/magnetometer",
       json{{"magnetometer", true}}, [](const json&) { return true; }},
      {"GetGroundTruthKinematics",
       [&]() { return env.drone->GetGroundTruthKinematics(&json_out); },
       "/Sim/SceneUnit/robots/Drone1/GetGroundTruthKinematics",
       json{{"kinematics", true}}, [](const json&) { return true; }},
      {"GetEstimatedKinematics",
       [&]() { return env.drone->GetEstimatedKinematics(&json_out); },
       "/Sim/SceneUnit/robots/Drone1/GetEstimatedKinematics",
       json{{"kinematics", true}}, [](const json&) { return true; }},
      {"GetImages",
       [&]() { return env.drone->GetImages("Camera1", {0, 1}, &json_out); },
       "/Sim/SceneUnit/robots/Drone1/sensors/Camera1/GetImages",
       json::array({json{{"image", true}}}),
       [](const json& p) { return p["image_type_ids"].size() == 2; }},
  };

  for (const auto& c : json_cases) {
    push(c.result);
    if (!Expect(c.call() == asc::Status::OK, c.name + " should succeed") ||
        !ExpectLastRequest(c.method, c.params, c.name)) {
      return false;
    }
  }

  push(json{{"ready_val", true}, {"ready_message", "ready"}});
  if (!Expect(env.drone->GetReadyState(&ready_state) == asc::Status::OK,
              "Drone::GetReadyState should succeed") ||
      !Expect(ready_state.is_ready, "Drone::GetReadyState should parse result") ||
      !ExpectLastRequest("/Sim/SceneUnit/robots/Drone1/GetReadyState",
                         "Drone::GetReadyState")) {
    return false;
  }

  push(0);
  if (!Expect(env.drone->GetLandedState(&landed_state) == asc::Status::OK,
              "Drone::GetLandedState should succeed") ||
      !Expect(landed_state == asc::LandedState::Landed,
              "Drone::GetLandedState should parse result") ||
      !ExpectLastRequest("/Sim/SceneUnit/robots/Drone1/GetLandedState",
                         "Drone::GetLandedState")) {
    return false;
  }

  push(TransformResultJson());
  if (!Expect(env.drone->GetGroundTruthPose(&transform_out) == asc::Status::OK,
              "Drone::GetGroundTruthPose should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/robots/Drone1/GetGroundTruthPose",
                         "Drone::GetGroundTruthPose")) {
    return false;
  }

  push(GeoPositionResultJson());
  if (!Expect(env.drone->GetGroundTruthGeoLocation(&geo_out) == asc::Status::OK,
              "Drone::GetGroundTruthGeoLocation should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/robots/Drone1/GetGroundTruthGeoLocation",
                         "Drone::GetGroundTruthGeoLocation")) {
    return false;
  }

  push(GeoPositionResultJson());
  if (!Expect(env.drone->GetEstimatedGeoLocation(&geo_out) == asc::Status::OK,
              "Drone::GetEstimatedGeoLocation should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/robots/Drone1/GetEstimatedGeoLocation",
                         "Drone::GetEstimatedGeoLocation")) {
    return false;
  }

  push(true);
  if (!Expect(env.drone->SetPose(asc::Transform(), true) == asc::Status::OK,
              "Drone::SetPose should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/robots/Drone1/SetPose",
                         [](const json& p) {
                           return p["reset_kinematics"] == true &&
                                  p.contains("pose");
                         },
                         "Drone::SetPose")) {
    return false;
  }

  push(true);
  if (!Expect(env.drone->CancelLastTask(&bool_out) == asc::Status::OK,
              "Drone::CancelLastTask should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/robots/Drone1/CancelLastTask",
                         "Drone::CancelLastTask")) {
    return false;
  }

  env.client->Disconnect();
  return true;
}

bool TestRoverAPIsUseExpectedServiceMethods() {
  TestEnvironment env;
  if (!CreateRover(&env)) return false;

  int id = 0;
  auto push = [&](const json& result) {
    fake_nngi::PushServiceResponse(SuccessResponse(id++, result));
  };

  bool bool_out = false;
  json json_out;

  struct RoverCase {
    std::string name;
    std::function<asc::Status()> call;
    std::string method;
    std::function<bool(const json&)> params;
  };

  const std::vector<RoverCase> cases = {
      {"Arm", [&]() { return env.rover->Arm(&bool_out); },
       "/Sim/SceneUnit/robots/Rover1/Arm", [](const json&) { return true; }},
      {"CanArm", [&]() { return env.rover->CanArm(&bool_out); },
       "/Sim/SceneUnit/robots/Rover1/CanArm", [](const json&) { return true; }},
      {"DisableAPIControl",
       [&]() { return env.rover->DisableAPIControl(&bool_out); },
       "/Sim/SceneUnit/robots/Rover1/DisableApiControl",
       [](const json&) { return true; }},
      {"Disarm", [&]() { return env.rover->Disarm(&bool_out); },
       "/Sim/SceneUnit/robots/Rover1/Disarm", [](const json&) { return true; }},
      {"EnableAPIControl",
       [&]() { return env.rover->EnableAPIControl(&bool_out); },
       "/Sim/SceneUnit/robots/Rover1/EnableApiControl",
       [](const json&) { return true; }},
      {"SetPose",
       [&]() { return env.rover->SetPose(asc::Transform(), true); },
       "/Sim/SceneUnit/robots/Rover1/SetPose",
       [](const json& p) { return p["reset_kinematics"] == true; }},
      {"SetRoverControls",
       [&]() { return env.rover->SetRoverControls(0.5f, 0.1f, 0.0f); },
       "/Sim/SceneUnit/robots/Rover1/SetRoverControls",
       [](const json& p) {
         return p["engine"] == 0.5f && p["steering_angle"] == 0.1f;
       }},
  };

  for (const auto& c : cases) {
    push(true);
    if (!Expect(c.call() == asc::Status::OK, c.name + " should succeed") ||
        !ExpectLastRequest(c.method, c.params, "Rover::" + c.name)) {
      return false;
    }
  }

  push(json{{"kinematics", true}});
  if (!Expect(env.rover->GetGroundTruthKinematics(&json_out) == asc::Status::OK,
              "Rover::GetGroundTruthKinematics should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/robots/Rover1/GetGroundTruthKinematics",
                         "Rover::GetGroundTruthKinematics")) {
    return false;
  }

  push(true);
  if (!Expect(env.rover->CancelLastTask(&bool_out) == asc::Status::OK,
              "Rover::CancelLastTask should succeed") ||
      !ExpectLastRequest("/Sim/SceneUnit/robots/Rover1/CancelLastTask",
                         "Rover::CancelLastTask")) {
    return false;
  }

  struct AsyncCase {
    std::string name;
    std::function<asc::AsyncResult()> call;
    std::string method;
    std::function<bool(const json&)> params;
  };

  const std::vector<AsyncCase> async_cases = {
      {"MoveToPositionAsync",
       [&]() { return env.rover->MoveToPositionAsync(1.0f, 2.0f, 3.0f, 4.0f); },
       "/Sim/SceneUnit/robots/Rover1/MoveToPosition",
       [](const json& p) {
         return p["x"] == 1.0f && p["y"] == 2.0f &&
                p["velocity"] == 3.0f;
       }},
      {"MoveByHeadingAsync",
       [&]() { return env.rover->MoveByHeadingAsync(1.0f, 2.0f, 3.0f); },
       "/Sim/SceneUnit/robots/Rover1/MoveByHeading",
       [](const json& p) {
         return p["heading"] == 1.0f && p["speed"] == 2.0f &&
                p["duration"] == 3.0f;
       }},
  };

  for (const auto& c : async_cases) {
    push(true);
    auto result = c.call();
    if (!Expect(result.Wait() == asc::Status::OK,
                "Rover::" + c.name + " should complete successfully") ||
        !ExpectLastRequest(c.method, c.params, "Rover::" + c.name)) {
      return false;
    }
  }

  env.client->Disconnect();
  return true;
}

bool TestEnvActorAPIsUseExpectedServiceMethods() {
  TestEnvironment env;
  if (!CreateEnvActor(&env)) return false;

  int id = 0;
  auto push = [&](const json& result) {
    fake_nngi::PushServiceResponse(SuccessResponse(id++, result));
  };

  struct EnvActorCase {
    std::string name;
    std::function<asc::Status()> call;
    std::string method;
    std::function<bool(const json&)> params;
  };

  const std::vector<EnvActorCase> cases = {
      {"SetTrajectory",
       [&]() {
         return env.env_actor->SetTrajectory("traj", true, 1.0f, 2.0f, 3.0f,
                                             4.0f, 0.1f, 0.2f, 0.3f);
       },
       "/Sim/SceneUnit/SetEnvActorTrajectory",
       [](const json& p) {
         return p["env_actor_name"] == "Actor1" && p["traj_name"] == "traj" &&
                p["to_loop"] == true;
       }},
      {"SetLinkRotationAngle",
       [&]() { return env.env_actor->SetLinkRotationAngle("link", 45.0f); },
       "/Sim/SceneUnit/SetEnvActorLinkRotAngle",
       [](const json& p) {
         return p["env_actor_name"] == "Actor1" && p["link_name"] == "link" &&
                p["angle_deg"] == 45.0f;
       }},
      {"SetLinkRotationRate",
       [&]() { return env.env_actor->SetLinkRotationRate("link", 5.0f); },
       "/Sim/SceneUnit/SetEnvActorLinkRotRate",
       [](const json& p) {
         return p["env_actor_name"] == "Actor1" && p["link_name"] == "link" &&
                p["rotation_deg_per_sec"] == 5.0f;
       }},
  };

  for (const auto& c : cases) {
    push(true);
    if (!Expect(c.call() == asc::Status::OK, c.name + " should succeed") ||
        !ExpectLastRequest(c.method, c.params, "EnvActor::" + c.name)) {
      return false;
    }
  }

  env.client->Disconnect();
  return true;
}

bool TestStaticSensorActorAPIsUseExpectedServiceMethods() {
  TestEnvironment env;
  if (!CreateStaticSensorActor(&env)) return false;

  fake_nngi::PushServiceResponse(
      SuccessResponse(0, json::array({json{{"image", true}}})));
  json json_out;
  if (!Expect(env.static_sensor_actor->GetImages("Camera1", {0, 1},
                                                 &json_out) ==
                  asc::Status::OK,
              "StaticSensorActor::GetImages should succeed") ||
      !ExpectLastRequest(
          "/Sim/SceneUnit/robots/Static1/sensors/Camera1/GetImages",
          [](const json& p) { return p["image_type_ids"].size() == 2; },
          "StaticSensorActor::GetImages")) {
    return false;
  }

  env.client->Disconnect();
  return true;
}

}  // namespace

int main() {
  int failed = 0;

  failed += TestRequestBeforeConnectFails() ? 0 : 1;
  failed += TestConnectAndRequest() ? 0 : 1;
  failed += TestGetBuildCommitHashUsesExpectedServiceMethod() ? 0 : 1;
  failed += TestRejectedResponseReturnsRejectedByServer() ? 0 : 1;
  failed += TestClientPublishSendsTopicMessage() ? 0 : 1;
  failed += TestClientTopicAndLifecycleAPIsUseExpectedMessages() ? 0 : 1;
  failed += TestWorldAPIsUseExpectedServiceMethods() ? 0 : 1;
  failed += TestDroneAPIsUseExpectedServiceMethods() ? 0 : 1;
  failed += TestRoverAPIsUseExpectedServiceMethods() ? 0 : 1;
  failed += TestEnvActorAPIsUseExpectedServiceMethods() ? 0 : 1;
  failed += TestStaticSensorActorAPIsUseExpectedServiceMethods() ? 0 : 1;

  if (failed == 0) {
    std::cout << "[OK] C++ client unit tests passed" << std::endl;
    return 0;
  }

  std::cerr << "[FAIL] " << failed << " C++ client unit test(s) failed" << std::endl;
  return 1;
}
