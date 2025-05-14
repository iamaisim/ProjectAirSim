// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_JSON_UTILS_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_JSON_UTILS_HPP_

#include <regex>
#include <string>
#include <vector>

#include "core_sim/config_json.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/message/airspeed_message.hpp"
#include "core_sim/message/barometer_message.hpp"
#include "core_sim/message/battery_message.hpp"
#include "core_sim/message/gps_message.hpp"
#include "core_sim/message/image_message.hpp"
#include "core_sim/message/imu_message.hpp"
#include "core_sim/message/kinematics_message.hpp"
#include "core_sim/message/magnetometer_message.hpp"
#include "core_sim/message/ready_state_message.hpp"
#include "core_sim/transforms/transform.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class JsonUtils {
 public:
  static const json GetArray(const json& data_json, const std::string& key,
                             const json& default_value = "[ ]"_json);

  // GetObject collides with Win SDK wingdi.h #define GetObject GetObjectA
  static const json GetJsonObject(const json& data_json, const std::string& key,
                                  const json& default_value = "{ }"_json);

  static bool GetBoolean(const json& data_json, const std::string& key,
                         const bool default_value = false);

  static const std::string GetIdentifier(const json& data_json,
                                         const std::string& key,
                                         const std::string& default_value = "");

  static int GetInteger(const json& data_json, const std::string& key,
                        const int default_value = 0);

  template <typename T>
  static T GetNumber(const json& data_json, const std::string& key,
                     const T default_value = 0) {
    return data_json.value(key, default_value);
  }

  static Quaternion GetQuaternion(
      const json& data_json, const std::string& key,
      const Quaternion& default_val = Quaternion::Identity());

  static const std::string GetString(const json& data_json,
                                     const std::string& key,
                                     const std::string& default_value = "");

  static Transform GetTransform(const json& data_json, const std::string& key,
                                const GeoPoint home_geo_point = GeoPoint());

  static Vector3 GetVector3(const json& data_json, const std::string& key,
                            const Vector3& default_val = Vector3(0, 0, 0));

  static Vector4 GetVector4(const json& data_json, const std::string& key,
                            const Vector4 default_val = Vector4(0, 0, 0, 0));

  static bool HasKey(const json& data_json, const std::string& key);

  static bool IsEmpty(const json& data_json);

  static bool IsEmptyArray(const json& data_json);

  static std::string GetTypeNameAsStr(json::value_t type_name);
};

// TODO This doesn't appear to be used because Vector3 is just an alias
// to Eigen::Vector3f which already has to_json() that outputs a list
inline void to_json(json& j, const Vector3& vector3) {
  j = json({{"x", vector3.x()}, {"y", vector3.y()}, {"z", vector3.z()}});
}

// TODO This doesn't appear to be used because Vector3 is just an alias
// to Eigen::Vector3f which already has to_json() that outputs a list
inline void from_json(const json& j, Vector3& vector3) {
  vector3 = Vector3(j.at("x"), j.at("y"), j.at("z"));
}

inline void to_json(json& j, const Transform& transform) {
  const auto& t = transform.translation_;
  const auto& q = transform.rotation_;
  j = json(
      {{"frame_id", transform.frame_id_},
       {"translation", {{"x", t.x()}, {"y", t.y()}, {"z", t.z()}}},
       {"rotation", {{"w", q.w()}, {"x", q.x()}, {"y", q.y()}, {"z", q.z()}}}});
}

inline void from_json(const json& j, Transform& transform) {
  const auto& t = j.at("translation");
  const auto& q = j.at("rotation");
  j.at("frame_id").get_to(transform.frame_id_);
  Vector3 translation = Vector3(t.at("x"), t.at("y"), t.at("z"));
  Quaternion rotation = Quaternion(q.at("w"), q.at("x"), q.at("y"), q.at("z"));
  transform = Transform(translation, rotation);
}

inline void from_json(const json& j, KinematicsMessage& kin_msg) {
  const auto& pose = j.at("pose");
  const auto& accn = j.at("accels");
  const auto& twist = j.at("twist");

  const auto& pose_t = pose.at("position");
  const auto& pose_o = pose.at("orientation");
  Vector3 translation = Vector3(pose_t.at("x"), pose_t.at("y"), pose_t.at("z"));
  Quaternion rotation = Quaternion(pose_o.at("w"), pose_o.at("x"),
                                   pose_o.at("y"), pose_o.at("z"));
  Pose kin_pose = Pose(translation, rotation);

  const auto& accn_l = accn.at("linear");
  const auto& accn_a = accn.at("angular");
  Vector3 accn_linear = Vector3(accn_l.at("x"), accn_l.at("y"), accn_l.at("z"));
  Vector3 accn_angular =
      Vector3(accn_a.at("x"), accn_a.at("y"), accn_a.at("z"));
  Accelerations kin_accn = Accelerations(accn_linear, accn_angular);

  const auto& twist_l = twist.at("linear");
  const auto& twist_a = twist.at("angular");
  Vector3 twist_linear =
      Vector3(twist_l.at("x"), twist_l.at("y"), twist_l.at("z"));
  Vector3 twist_angular =
      Vector3(twist_a.at("x"), twist_a.at("y"), twist_a.at("z"));
  Twist kin_twist = Twist(twist_linear, twist_angular);

  Kinematics kin = Kinematics(kin_pose, kin_twist, kin_accn);
  TimeNano time = j.at("time_stamp");

  kin_msg = KinematicsMessage(time, kin);
}

// TODO this also doesn't seem to be used since it's an alias to
// Eigen::Quaternion (see Vector3 above)
inline void to_json(json& j, const Quaternion& q) {
  j = json({{"w", q.w()}, {"x", q.x()}, {"y", q.y()}, {"z", q.z()}});
}

// TODO this also doesn't seem to be used since it's an alias to
// Eigen::Quaternion (see Vector3 above)
inline void from_json(const json& j, Quaternion& q) {
  q = Quaternion(j.at("w"), j.at("x"), j.at("y"), j.at("z"));
}

inline void to_json(json& j, const Pose& pose) {
    json json_o = {{"w", pose.orientation.w()},
                   {"x", pose.orientation.x()},
                   {"y", pose.orientation.y()},
                   {"z", pose.orientation.z()}};
    json json_p = {{"x", pose.position.x()},
                   {"y", pose.position.y()},
                   {"z", pose.position.z()}};
  j = json({{"position", json_p},
            {"orientation", json_o}});
}

inline void from_json(const json& j, Pose& pose) {
    auto &j_o = j.at("orientation");
    auto &j_p = j.at("position");


    pose.position = Vector3(j_p.at("x"), j_p.at("y"), j_p.at("z"));
    pose.orientation = Quaternion(j_o.at("w"), j_o.at("x"), j_o.at("y"), j_o.at("z"));
}

inline void to_json(json& j, const AirspeedMessage& airspeed_msg) {
  j = airspeed_msg.getData();
}
inline void from_json(const json& j, AirspeedMessage& airspeed_msg) {
  airspeed_msg = AirspeedMessage(j.at("time_stamp"), j.at("diff_pressure"));
}

inline void to_json(json& j, const BarometerMessage& barometer_msg) {
  j = barometer_msg.getData();
}
inline void from_json(const json& j, BarometerMessage& barometer_msg) {
  barometer_msg = BarometerMessage(j.at("time_stamp"), j.at("altitude"),
                                   j.at("pressure"), j.at("qnh"));
}

inline void to_json(json& j, const MagnetometerMessage& magnetometer_msg) {
  j = magnetometer_msg.getData();
}

inline void from_json(const json& j, MagnetometerMessage& magnetometer_msg) {
  auto time_stamp = j.at("time_stamp");
  auto mfb = j.at("magnetic_field_body");
  Vector3 magnetic_field_body = Vector3(mfb.at("x"), mfb.at("y"), mfb.at("z"));
  std::vector<float> magnetic_field_covariance =
      j.at("magnetic_field_covariance");
  magnetometer_msg = MagnetometerMessage(time_stamp, magnetic_field_body,
                                         magnetic_field_covariance);
}

inline void to_json(json& j, const GpsMessage& gps_msg) {
  j = gps_msg.getData();
}

inline void from_json(const json& j, GpsMessage& gps_msg) {
  auto v = j.at("velocity");
  Vector3 velocity = Vector3(v.at("x"), v.at("y"), v.at("z"));
  gps_msg =
      GpsMessage(j.at("time_stamp"), j.at("time_utc_milli"), j.at("latitude"),
                 j.at("longitude"), j.at("altitude"), j.at("epv"), j.at("eph"),
                 j.at("position_cov_type"), j.at("fix_type"), velocity);
}

inline void to_json(json& j, const BatteryStateMessage& battery_msg) {
  j = battery_msg.getData();
}

inline void from_json(const json& j, BatteryStateMessage& battery_msg) {
  battery_msg = BatteryStateMessage(
      j.at("time_stamp"), j.at("battery_remaining"),
      j.at("estimated_time_remaining"), j.at("battery_charge_state"));
}

inline void to_json(json& j, const ImuMessage& imu_msg) {
  j = imu_msg.getData();
}

inline void from_json(const json& j, ImuMessage& imu_msg) {
  auto time_stamp = j.at("time_stamp");
  auto orientation = j.at("orientation");
  auto angular_velocity = j.at("angular_velocity");
  auto linear_acceleration = j.at("linear_acceleration");
  imu_msg = ImuMessage(
      time_stamp,
      Quaternion(orientation.at("w"), orientation.at("x"), orientation.at("y"),
                 orientation.at("z")),
      Vector3(angular_velocity.at("x"), angular_velocity.at("y"),
              angular_velocity.at("z")),
      Vector3(linear_acceleration.at("x"), linear_acceleration.at("y"),
              linear_acceleration.at("z")));
}

inline void to_json(json& j, const KinematicsMessage& kin_msg) {
  Kinematics kin = kin_msg.GetKinematics();

  json position_json = {{"x", kin.pose.position.x()},
                        {"y", kin.pose.position.y()},
                        {"z", kin.pose.position.z()}};
  json orientation_json = {{"w", kin.pose.orientation.w()},
                           {"x", kin.pose.orientation.x()},
                           {"y", kin.pose.orientation.y()},
                           {"z", kin.pose.orientation.z()}};

  json twist_linear_json = {{"x", kin.twist.linear.x()},
                            {"y", kin.twist.linear.y()},
                            {"z", kin.twist.linear.z()}};
  json twist_angular_json = {{"x", kin.twist.angular.x()},
                             {"y", kin.twist.angular.y()},
                             {"z", kin.twist.angular.z()}};

  json accels_linear_json = {{"x", kin.accels.linear.x()},
                             {"y", kin.accels.linear.y()},
                             {"z", kin.accels.linear.z()}};
  json accels_angular_json = {{"x", kin.accels.angular.x()},
                              {"y", kin.accels.angular.y()},
                              {"z", kin.accels.angular.z()}};

  j = json(
      {{"time_stamp", kin_msg.GetTimeStamp()},
       {"pose",
        {{"position", position_json}, {"orientation", orientation_json}}},
       {"twist",
        {{"linear", twist_linear_json}, {"angular", twist_angular_json}}},
       {"accels",
        {{"linear", accels_linear_json}, {"angular", accels_angular_json}}}});
}

inline void to_json(json& j, const BBox2D& bbox2D) {
  json center_json = {{"x", bbox2D.center.x()}, {"y", bbox2D.center.y()}};
  json size_json = {{"x", bbox2D.size.x()}, {"y", bbox2D.size.y()}};

  j = json({{"center", center_json}, {"size", size_json}});
}

inline void to_json(json& j, const BBox3D& bbox3D) {
  json center_json = {{"x", bbox3D.center.x()},
                      {"y", bbox3D.center.y()},
                      {"z", bbox3D.center.z()}};

  json orientation_json = {{"w", bbox3D.quaternion.w()},
                           {"x", bbox3D.quaternion.x()},
                           {"y", bbox3D.quaternion.y()},
                           {"z", bbox3D.quaternion.z()}};
  json size_json = {
      {"x", bbox3D.size.x()}, {"y", bbox3D.size.y()}, {"z", bbox3D.size.z()}};

  j = json({{"center", center_json},
            {"size", size_json},
            {"quaternion", orientation_json}});
}

inline void to_json(json& j, const Annotation& a) {
  json proj_bbox_json_2D = json::array();
  json proj_bbox_json_3D = json::array();
  for (const auto& point : a.bbox3d_in_image_space) {
    proj_bbox_json_2D.push_back({{"x", point.x()}, {"y", point.y()}});
  }
  for (const auto& point : a.bbox3d_in_projection_space) {
    proj_bbox_json_3D.push_back(
        {{"x", point.x()}, {"y", point.y()}, {"z", point.z()}});
  }
  j = json({{"object_id", a.object_id},
            {"bbox2d", a.bbox2D},
            {"bbox3d", a.bbox3D},
            {"bbox3d_in_image_space", proj_bbox_json_2D},
            {"bbox3d_in_projection_space", proj_bbox_json_3D}});
}

inline void to_json(json& j, const GeoPoint& g) {
  j = json({{"latitude", g.latitude},
            {"longitude", g.longitude},
            {"altitude", g.altitude}});
}

inline void from_json(const json& j, ReadyStateMessage& ready_state_message) {
  ready_state_message = ReadyStateMessage(j.at("time_stamp"), j.at("ready_val"),
                                          j.at("ready_message"));
}

inline void to_json(json& j, const ReadyStateMessage& ready_state_message) {
  j = json({{"time_stamp", ready_state_message.GetTimeStamp()},
            {"ready_val", ready_state_message.GetReadyVal()},
            {"ready_message", ready_state_message.GetReadyMessage()}});
}

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_JSON_UTILS_HPP_
