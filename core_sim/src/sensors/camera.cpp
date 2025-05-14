// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "core_sim/sensors/camera.hpp"

#include <map>
#include <memory>
#include <queue>
#include <string>

#include "algorithms.hpp"
#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "json.hpp"
#include "onnxruntime_cxx_api.h"
#include "sensor_impl.hpp"
#include "tensorrt_provider_factory.h"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class Camera::Loader {
 public:
  explicit Loader(Camera::Impl& impl);

  void Load(const json& json);

 private:
  void LoadCaptureInterval(const json& json);
  void InitializeCaptureSettings();
  void LoadCaptureSettings(const json& json);
  void LoadCaptureSetting(const json& json);
  void InitializeNoiseSettings();
  void LoadNoiseSettings(const json& json);
  void LoadNoiseSetting(const json& json);
  void LoadGimbalSetting(const json& json);
  void LoadOriginSetting(const json& json);
  void LoadAnnotationSettings(const json& json);
  void LoadOnnxModelSettings(const json& json);

  Camera::Impl& impl;
};

class Camera::Impl : public SensorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  void Initialize(const Kinematics&, const Environment&);

  void Update(const TimeNano, const TimeNano);

  void CreateTopics();

  const CameraSettings& GetCameraSetting() const;

  const Transform& GetOrigin() const;

  TimeNano GetCapturedImagesTimestamp();

  json GetImages(std::vector<int> image_type_ids);

  bool LookAtObject(const std::string& object_name, bool wait_for_pose_update);

  bool DrawFrustum(const int image_type, const bool to_enable);

  std::vector<uint8_t> RunOnnxModelOnImages(ImageMessage imgMsg);

  bool SetPose(const Transform& new_pose, bool wait_for_pose_update);

  bool SetFocalLength(int image_type_id, float focal_length);

  bool SetChromaticAberrationIntensity(int image_type_id, float intensity);

  bool SetDepthOfFieldFocalRegion(int image_type_id, float focal_length);

  bool SetDepthOfFieldTransitionRegion(int image_type_id,
                                       float transition_threshold);

  bool SetFieldOfView(int image_type_id, float field_of_view);

  bool SetAperture(float aperture);

  bool IsPoseUpdatePending() const;

  bool IsSettingsUpdatePending() const;

  const std::string GetLookAtObject() const;

  const bool GetFrustumEnabled() const;

  const int GetFrustumCaptureType() const;

  Pose GetRay(ImageType image_type, const Vector2& image_position) const;

  bool HasGimbal() const;

  const std::string& GetGimbalId() const;

  const Transform& GetDesiredPose() const;

  void MarkPoseUpdateAsCompleted();

  void MarkSettingsUpdateAsCompleted();

  const bool ResetCameraPose(bool wait_for_pose_update);

  void OnBeginUpdate() override;

  void PublishImages(std::map<ImageType, ImageMessage>&& images);

  bool GetCaptureQueueFrontImageTime(TimeNano* timestamp);

  bool WaitForEarlierCaptures(const TimeNano& timestep, int sleep_interval_ns);

  void AddRequestToCaptureQueue(TimeNano time_stamp);

  bool IsCaptureQueueFull() const;

  void RegisterServiceMethods();

  void OnEndUpdate() override;

  bool HasSubscribers(int iimage_type) const;

 private:
  void OnSubscribed(const Topic& topic, bool is_subscribed);
  void OnSubscribed_Onnx(const Topic& topic, bool is_subscribed);

 private:
  friend class Camera::Loader;
  friend class ImageGenerationOverride;

  // One entry of this struct per image type in image_entries_
  struct ImageEntry {
    Topic image_topic;  // Topic for image type images
    Topic info_topic;   // Topic for iamge type info
    std::atomic<bool> fimage_is_subscribed =
        false;  // True if image topic is subscribed to, false otherwise
    std::atomic<bool> finfo_is_subscribed =
        false;  // True if info topic is subscribed to, false otherwise
    std::atomic<int> cimage_force_generate =
        0;  // >0 if we want to generate an image regardless whether the
            // topic is being subscribed to

    void Set(Topic&& image_topic_in, Topic&& info_topic_in) {
      this->image_topic = std::move(image_topic_in);
      this->info_topic = std::move(info_topic_in);
    }
  };  // struct ImageEntry

  // Image entry array with some conveniences
  class ImageEntries {
   public:
    auto begin(void) { return std::begin(image_entries_); }
    auto begin(void) const { return std::begin(image_entries_); }
    auto end(void) { return std::end(image_entries_); }
    auto end(void) const { return std::end(image_entries_); }
    auto size(void) const { return (int)ImageType::kCount; }
    bool is_valid_index(int i) const { return ((i >= 0) && (i < size())); }
    bool is_valid_index(ImageType image_type) const {
      return ((MathUtils::ToNumeric(image_type) >= 0) &&
              (MathUtils::ToNumeric(image_type) < size()));
    }

    ImageEntry& operator[](int i) { return image_entries_[i]; }
    ImageEntry& operator[](ImageType image_type) {
      return image_entries_[MathUtils::ToNumeric(image_type)];
    }
    const ImageEntry& operator[](int i) const { return image_entries_[i]; }
    const ImageEntry& operator[](ImageType image_type) const {
      return image_entries_[MathUtils::ToNumeric(image_type)];
    }  // class ImageEntries

   private:
    ImageEntry image_entries_[(int)ImageType::kCount];  // Image type entries
  };                                                    // class ImageEntries

  // This helper class adds an image generation override to an image type
  // and removes it upon destruction.  By instantiating an instance on the
  // stack, it ensures the overrides are removed even in case of an exception
  // being thrown.
  class ImageGenerationOverride {
   public:
    ImageGenerationOverride(Camera::Impl* pcamera_impl,
                            size_t centry_anticipated)
        : pcamera_impl_(pcamera_impl), vec_iimage_type_requested_() {
      vec_iimage_type_requested_.reserve(centry_anticipated);
    }

    ~ImageGenerationOverride() {
      for (const int iimage_type : vec_iimage_type_requested_)
        --pcamera_impl_->image_entries_[iimage_type].cimage_force_generate;
    }

    // Enable image generation override for the specified image type
    void Add(int iimage_type) {
      ++pcamera_impl_->image_entries_[iimage_type].cimage_force_generate;
      vec_iimage_type_requested_.push_back(iimage_type);
    }

   private:
    std::vector<int> vec_iimage_type_requested_;  // Image types for which an
                                                  // override was requested
    Camera::Impl* pcamera_impl_;  // Pointer to owner of image_entries_ array
  };                              // class ImageGenerationOverride

  // Onnx runtime objects
  struct Onnx {
    std::unique_ptr<Ort::AllocatorWithDefaultOptions> allocator = nullptr;
    Ort::Env env = Ort::Env(nullptr);
    Ort::MemoryInfo memory_info = Ort::MemoryInfo(nullptr);
    Ort::SessionOptions session_options = Ort::SessionOptions(nullptr);
    Ort::Session session = Ort::Session(nullptr);
    ImageEntry post_process_image_entry;  // Topic entry for publishing
                                          // Onnx-generated image
  };                                      // struct Onnx

 private:
  Camera::Loader loader;  // Config loader

  CameraSettings camera_settings;  // Camera settings loaded from config

  ImageEntries image_entries_;  // Image type entries
  Onnx onnx_;                   // Onnx-related data

  std::queue<TimeNano> capture_queue;  // Queue of image capture times
  std::map<ImageType, ImageMessage>
      captured_images;  // Mapping from image type to image captured from scene
                        // render

  std::atomic<bool> pose_update_pending =
      false;                       // Camera has a new pose ready to apply
  Transform desired_current_pose;  // Current pose

  std::string object_to_look_at =
      "";                         // Name of object camera gimbal is tracking
  bool enable_frustum = false;    // If true, an illustration of the camera
  int frustum_capture_type = -1;  // The image type's frustum to illustration
                                  // frustum is drawn in the scene
  std::atomic<bool> settings_update_pending =
      false;  // If true, one or more camera settings have been changed

 private:
  // Capture queue max size is used to throttle the number of image processing
  // async threads on the Unreal side
  static constexpr int kCaptureQueueMaxSize = 24;

  // Timeout limit for delaying image publishing to keep them in sync with the
  // order that the image captures were requested in
  static constexpr TimeMilli kTimeoutPublishSync = 5000;

  // Timeout limit for waiting for new images after receiving a GetImages()
  // service method request
  static constexpr TimeMilli kTimeoutGetCaptures = 5000;

  // Timeout limit for waiting for renderer to process camera pose updates
  static constexpr int kTimeoutCameraPoseUpdate = 1000;
};  // class camera

Camera::Camera() : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

Camera::Camera(const std::string& id, bool is_enabled,
               const std::string& parent_link, const Logger& logger,
               const TopicManager& topic_manager,
               const std::string& parent_topic_path,
               const ServiceManager& service_manager,
               const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(new Camera::Impl(
          id, is_enabled, parent_link, logger, topic_manager, parent_topic_path,
          service_manager, state_manager))) {}

void Camera::Load(ConfigJson config_json) {
  static_cast<Camera::Impl*>(pimpl.get())->Load(config_json);
}

void Camera::Initialize(const Kinematics& kinematics,
                        const Environment& environment) {
  static_cast<Camera::Impl*>(pimpl.get())->Initialize(kinematics, environment);
}

void Camera::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  static_cast<Camera::Impl*>(pimpl.get())->Update(sim_time, sim_dt_nanos);
}

const CameraSettings& Camera::GetCameraSettings() const {
  return static_cast<Camera::Impl*>(pimpl.get())->GetCameraSetting();
}

void Camera::BeginUpdate() {
  static_cast<Camera::Impl*>(pimpl.get())->BeginUpdate();
}

void Camera::PublishImages(std::map<ImageType, ImageMessage>&& images) {
  static_cast<Camera::Impl*>(pimpl.get())->PublishImages(std::move(images));
}

void Camera::AddRequestToCaptureQueue(TimeNano time_stamp) {
  static_cast<Camera::Impl*>(pimpl.get())->AddRequestToCaptureQueue(time_stamp);
}

bool Camera::HasSubscribers(int iimage_type) const {
  return static_cast<Camera::Impl*>(pimpl.get())->HasSubscribers(iimage_type);
}

bool Camera::IsCaptureQueueFull() const {
  return static_cast<Camera::Impl*>(pimpl.get())->IsCaptureQueueFull();
}

bool Camera::IsPoseUpdatePending() const {
  return static_cast<Camera::Impl*>(pimpl.get())->IsPoseUpdatePending();
}

bool Camera::IsSettingsUpdatePending() const {
  return static_cast<Camera::Impl*>(pimpl.get())->IsSettingsUpdatePending();
}

const std::string Camera::GetLookAtObject() const {
  return static_cast<Camera::Impl*>(pimpl.get())->GetLookAtObject();
}

const bool Camera::GetFrustumEnabled() const {
  return static_cast<Camera::Impl*>(pimpl.get())->GetFrustumEnabled();
}

const int Camera::GetFrustumCaptureType() const {
  return static_cast<Camera::Impl*>(pimpl.get())->GetFrustumCaptureType();
}

Pose Camera::GetRay(ImageType image_type, const Vector2& image_position) const {
  return static_cast<Camera::Impl*>(pimpl.get())
      ->GetRay(image_type, image_position);
}

bool Camera::SetPose(const Transform& new_pose, bool wait_for_pose_update) {
  return static_cast<Camera::Impl*>(pimpl.get())
      ->SetPose(new_pose, wait_for_pose_update);
}

bool Camera::HasGimbal() const {
  return static_cast<Camera::Impl*>(pimpl.get())->HasGimbal();
}

const std::string& Camera::GetGimbalId() const {
  return static_cast<Camera::Impl*>(pimpl.get())->GetGimbalId();
};

const Transform& Camera::GetDesiredPose() const {
  return static_cast<Camera::Impl*>(pimpl.get())->GetDesiredPose();
}

void Camera::MarkPoseUpdateAsCompleted() {
  return static_cast<Camera::Impl*>(pimpl.get())->MarkPoseUpdateAsCompleted();
}

void Camera::MarkSettingsUpdateAsCompleted() {
  return static_cast<Camera::Impl*>(pimpl.get())
      ->MarkSettingsUpdateAsCompleted();
}

const bool Camera::ResetCameraPose(bool wait_for_pose_update) {
  return static_cast<Camera::Impl*>(pimpl.get())
      ->ResetCameraPose(wait_for_pose_update);
}

void Camera::EndUpdate() {
  static_cast<Camera::Impl*>(pimpl.get())->EndUpdate();
}

// class camera::impl

Camera::Impl::Impl(const std::string& id, bool is_enabled,
                   const std::string& parent_link, const Logger& logger,
                   const TopicManager& topic_manager,
                   const std::string& parent_topic_path,
                   const ServiceManager& service_manager,
                   const StateManager& state_manager)
    : SensorImpl(SensorType::kCamera, id, is_enabled, parent_link,
                 Constant::Component::camera, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      loader(*this) {
  SetTopicPath();
  CreateTopics();
}

void Camera::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader.Load(json);
}

void Camera::Impl::Initialize(const Kinematics& kinematics,
                              const Environment& environment) {
  //! Ground-truth kinematics and environment info insn't used
  //! by the camera impl
  if (camera_settings.post_process_model_settings.enabled) {
    onnx_.env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "onnx");
    onnx_.session_options = Ort::SessionOptions();
    onnx_.memory_info = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    onnx_.allocator = std::make_unique<Ort::AllocatorWithDefaultOptions>();
  }
}

void Camera::Impl::Update(const TimeNano sim_time,
                          const TimeNano sim_dt_nanos) {
  //! Camera updates on render ticks from the Rendering engine
  //! (UE) and therefore does not execute any op specifically at scene/physics
  //! ticks
}

void Camera::Impl::CreateTopics() {
  image_entries_[ImageType::kScene].Set(
      Topic("scene_camera", topic_path_, TopicType::kPublished, 60,
            MessageType::kImage),
      Topic("scene_camera_info", topic_path_, TopicType::kPublished, 60,
            MessageType::kCameraInfo));
  image_entries_[ImageType::kDepthPerspective].Set(
      Topic("depth_camera", topic_path_, TopicType::kPublished, 60,
            MessageType::kImage),
      Topic("depth_camera_info", topic_path_, TopicType::kPublished, 60,
            MessageType::kCameraInfo));

  image_entries_[ImageType::kDepthPlanar].Set(
      Topic("depth_planar_camera", topic_path_, TopicType::kPublished, 60,
            MessageType::kImage),
      Topic("depth_planar_camera_info", topic_path_, TopicType::kPublished, 60,
            MessageType::kCameraInfo));

  image_entries_[ImageType::kSegmentation].Set(
      Topic("segmentation_camera", topic_path_, TopicType::kPublished, 60,
            MessageType::kImage),
      Topic("segmentation_camera_info", topic_path_, TopicType::kPublished, 60,
            MessageType::kCameraInfo));

  image_entries_[ImageType::kDepthVis].Set(
      Topic("depth_vis_camera", topic_path_, TopicType::kPublished, 60,
            MessageType::kImage),
      Topic("depth_vis_camera_info", topic_path_, TopicType::kPublished, 60,
            MessageType::kCameraInfo));

  image_entries_[ImageType::kDisparityNormalized].Set(
      Topic("disparity_normalized_camera", topic_path_, TopicType::kPublished,
            60, MessageType::kImage),
      Topic("disparity_normalized_camera", topic_path_, TopicType::kPublished,
            60, MessageType::kCameraInfo));

  image_entries_[ImageType::kSurfaceNormals].Set(
      Topic("surface_normals_camera", topic_path_, TopicType::kPublished, 60,
            MessageType::kImage),
      Topic("surface_normals_camera_info", topic_path_, TopicType::kPublished,
            60, MessageType::kCameraInfo));

  onnx_.post_process_image_entry.image_topic =
      Topic("post_process_onnx_image", topic_path_, TopicType::kPublished, 60,
            MessageType::kImage);
}

const CameraSettings& Camera::Impl::GetCameraSetting() const {
  return camera_settings;
}

TimeNano Camera::Impl::GetCapturedImagesTimestamp() {
  std::lock_guard<std::mutex> lock(update_lock_);
  if (!captured_images.empty()) {
    // All image timestamps in a pack of captured_images should be the same, so
    // just return the timestamp from the first one
    return captured_images.begin()->second.GetTimestamp();
  } else {
    return -1;
  }
}

json Camera::Impl::GetImages(std::vector<int> image_type_ids) {
  if (image_type_ids.empty()) return json({});

  json images_json = json({});
  ImageGenerationOverride image_generation_override(this,
                                                    image_type_ids.size());
  auto sleep_interval_ns =
      static_cast<int>(camera_settings.capture_interval * 1.0e9f);
  TimeNano t_now;
  std::chrono::steady_clock::time_point t_start;

  // Force generation of the requested image types even though the image types
  // are not subscribed to.  The overrides are automatically removed when
  // the image_generation_override object goes out of scope.
  for (const int iimage_type : image_type_ids) {
    if (image_entries_.is_valid_index(iimage_type))
      image_generation_override.Add(iimage_type);
  }

  // Wait until the captured image's timestamp is past the time at the
  // GetImages request.
  t_start = std::chrono::steady_clock::now();
  t_now = SimClock::Get()->NowSimNanos();

  // logger_.LogVerbose(name_,
  //                    "[%s] Waiting to receive captured_images newer than "
  //                    "when GetImages request was received at simtime=%ld.",
  //                    id_.c_str(), t_now);
  while (GetCapturedImagesTimestamp() < t_now) {
    // Check for timeout condition
    auto t_dur = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t_start);
    if (t_dur.count() > kTimeoutGetCaptures) {
      logger_.LogError(
          name_, "[%s] GetImages() Timeout waiting for new captured_images.",
          id_.c_str());
      throw Error("GetImages() Timeout waiting for new captured_images.");
    }

    std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_interval_ns));
  }

  {
    std::lock_guard<std::mutex> lock(update_lock_);

    for (const int img_type_int : image_type_ids) {
      auto it_image =
          captured_images.find(static_cast<ImageType>(img_type_int));
      if (it_image != captured_images.end()) {
        const ImageMessage& image_msg = (*it_image).second;
        // Confirm every image in this pack has the new-enough timestamp
        if (image_msg.GetTimestamp() < t_now) {
          logger_.LogWarning(name_,
                             "[%s] Image found with mismatched old timestamp.",
                             id_.c_str());
        }
        images_json[std::to_string(img_type_int)] = image_msg.GetData();
      } else {
        logger_.LogWarning(
            name_, "[%s] Requested image type not found in captured_images.",
            id_.c_str());
      }
    }
  }

  return images_json;
}

bool Camera::Impl::HasSubscribers(int iimage_type) const {
  if ((iimage_type < 0) || (iimage_type >= image_entries_.size())) {
    logger_.LogError(
        name_, "[%s] Unsupported ImageType specified when querying subscribers",
        id_.c_str());
    throw Error("Unsupported ImageType specified when querying subscribers");
  }

  auto& image_entry = image_entries_[iimage_type];

  return (image_entry.fimage_is_subscribed || image_entry.finfo_is_subscribed ||
          (image_entry.cimage_force_generate > 0));
}

std::vector<uint8_t> Camera::Impl::RunOnnxModelOnImages(ImageMessage imgMsg) {
  auto& encoding = imgMsg.GetEncoding();
  auto& input_img_data = imgMsg.GetPixelVector();
  auto input_img_width = imgMsg.GetWidth();
  auto input_img_height = imgMsg.GetHeight();

  try {
    if (!camera_settings.post_process_model_settings.session_initialized) {
      logger_.LogVerbose(name_, "[%s] Entered onnx section'.", id_.c_str());
      if (camera_settings.post_process_model_settings.execution_provider ==
          "cuda") {
        logger_.LogVerbose(name_, "Trying to add CUDA EP since it is enabled.");
        OrtSessionOptionsAppendExecutionProvider_CUDA(onnx_.session_options, 0);
        logger_.LogVerbose(name_, "onnx CUDA session declared");
      } else if (camera_settings.post_process_model_settings
                     .execution_provider == "tensorrt") {
        OrtSessionOptionsAppendExecutionProvider_Tensorrt(onnx_.session_options,
                                                          0);
        OrtSessionOptionsAppendExecutionProvider_CUDA(onnx_.session_options, 0);
        logger_.LogVerbose(name_, "onnx TensorRT session declared");
      }
      logger_.LogVerbose(name_, "onnx Creating session");
      auto model_file = camera_settings.post_process_model_settings.filepath;

#ifdef _WIN32
      auto model_file_wstr = std::wstring(model_file.begin(), model_file.end());
      onnx_.session = Ort::Session(onnx_.env, model_file_wstr.c_str(),
                                   onnx_.session_options);
#else
      onnx_.session =
          Ort::Session(onnx_.env, model_file.c_str(), onnx_.session_options);
#endif
      logger_.LogVerbose(name_, "onnx Created session");
      camera_settings.post_process_model_settings.session_initialized = true;
    }

    Ort::Value input_tensor{nullptr};
    auto input_shape = onnx_.session.GetInputTypeInfo(0)
                           .GetTensorTypeAndShapeInfo()
                           .GetShape();

    // Infer order of input required ChannelsxHeightxWidth vs
    // HeightxWidthxChannels by looking at which of the input shapes is "3" -
    // assume we only pass in 3 channel images for now.

    // By default, input is 4 dimensional, Batches x C x H x W and we care only
    // about the second axis.
    auto first_img_axis = 1;

    // If input is three dimensional, there is no channel for batches
    if (input_shape.size() == 3) {
      first_img_axis = 0;
    }

    std::vector<float> input;
    auto img_size = input_img_height * input_img_width;
    if (encoding == "32FC1") {
      input = std::vector<float>(input_img_data.begin(), input_img_data.end());
    } else if (input_shape[first_img_axis] == 3) {
      // The input needs to be 3xHxW so we need to restructure our linear array
      // This particular implementation stacks them in RGB order since it's very
      // common for most models.
      std::vector<float> reordered(input_img_data.size());
      for (int i = 0; i < img_size; i++) {
        reordered[img_size * 2 + i] = input_img_data[3 * i] / 255.0;
        reordered[img_size + i] = input_img_data[3 * i + 1] / 255.0;
        reordered[i] = input_img_data[3 * i + 2] / 255.0;
      }
      input = reordered;
    } else {
      // Should we restructure normal ones to be in RGB format instead of our
      // BGR?
      input = std::vector<float>(input_img_data.begin(), input_img_data.end());
    }

    auto allocator_ptr = onnx_.allocator.get();
    if (allocator_ptr == nullptr) {
      throw Error("Invalid onnx allocator when trying to run model on image.");
    }

    const char* input_names[] = {onnx_.session.GetInputName(0, *allocator_ptr)};
    const char* output_names[] = {
        onnx_.session.GetOutputName(0, *allocator_ptr)};

    input_tensor = Ort::Value::CreateTensor<float>(
        onnx_.memory_info, input.data(), input.size(), input_shape.data(),
        input_shape.size());

    auto outputs = onnx_.session.Run(Ort::RunOptions{nullptr}, input_names,
                                     &input_tensor, 1, output_names, 1);

    auto output_img_size =
        outputs[0].GetTensorTypeAndShapeInfo().GetElementCount();
    auto output_img_data = outputs[0].GetTensorMutableData<float>();
    std::vector<uint8_t> result_vec(
        reinterpret_cast<uint8_t*>(output_img_data),
        reinterpret_cast<uint8_t*>(output_img_data) +
            output_img_size * sizeof(*output_img_data));
    // Successfully processed image with onnx model. Return the image result.
    return result_vec;
  } catch (const std::exception& e) {
    logger_.LogWarning(name_, "Exception in onnx: [%s] '", e.what());
  }
  // Exception was caught during onnx processing, return the unmodified image.
  return input_img_data;
}

bool Camera::Impl::LookAtObject(const std::string& object_name,
                                bool wait_for_pose_update) {
  if (object_name.empty()) {
    ResetCameraPose(wait_for_pose_update);
    return true;
  }

  {
    std::lock_guard<std::mutex> lock(update_lock_);
    object_to_look_at = object_name;
    pose_update_pending = true;
  }  // scope block for lock_guard

  if (wait_for_pose_update) {
    // Wait for renderer to set pose_update_pending back to false once the new
    // camera pose has been processed
    for (int i = 0;
         i < kTimeoutCameraPoseUpdate && pose_update_pending.load() == true;
         ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (pose_update_pending) {
      logger_.LogWarning(name_,
                         "[LookAtObject] Timeout waiting for camera "
                         "pose_update_pending to clear.");
      return false;
    }
  }

  return true;
}

bool Camera::Impl::DrawFrustum(const int image_type, const bool to_enable) {
  std::lock_guard<std::mutex> lock(update_lock_);
  enable_frustum = to_enable;
  frustum_capture_type = image_type;
  return true;
}

void Camera::Impl::OnSubscribed(const Topic& topic, bool is_subscribed) {
  auto& path = topic.GetPath();

  for (auto& image_entry : image_entries_) {
    if (path == image_entry.image_topic.GetPath()) {
      image_entry.fimage_is_subscribed = is_subscribed;
      // logger_.LogVerbose(
      //     name_, "Got notification of %s to camera %s image type %s image'.",
      //     is_subscribed ? "subscribe" : "unsubscribe", id_.c_str(),
      //     topic.GetName().c_str());
      break;
    } else if (path == image_entry.info_topic.GetPath()) {
      image_entry.finfo_is_subscribed = is_subscribed;
      // logger_.LogVerbose(
      //     name_, "Got notification of %s to camera %s image type %s info'.",
      //     is_subscribed ? "subscribe" : "unsubscribe", id_.c_str(),
      //     topic.GetName().c_str());
      break;
    }
  }
}

void Camera::Impl::OnSubscribed_Onnx(const Topic& topic, bool is_subscribed) {
  onnx_.post_process_image_entry.fimage_is_subscribed = is_subscribed;
}

// Sets the pose update
bool Camera::Impl::SetPose(const Transform& new_pose,
                           bool wait_for_pose_update) {
  {
    std::lock_guard<std::mutex> lock(update_lock_);
    object_to_look_at.clear();
    desired_current_pose = new_pose;
    pose_update_pending = true;
  }  // scope block for lock_guard

  if (wait_for_pose_update) {
    // Wait for renderer to set pose_update_pending back to false once the new
    // camera pose has been processed
    for (int i = 0;
         i < kTimeoutCameraPoseUpdate && pose_update_pending.load() == true;
         ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (pose_update_pending) {
      logger_.LogWarning(name_,
                         "[SetPose] Timeout waiting for camera "
                         "pose_update_pending to clear.");
      return false;
    }
  }

  return true;
}

bool Camera::Impl::SetFocalLength(int image_type_id, float focal_length) {
  SetFieldOfView(image_type_id, TransformUtils::ToDegrees(
                                    2 * atan2(camera_settings.sensor_width,
                                              2 * focal_length)));
  return true;
}

bool Camera::Impl::SetChromaticAberrationIntensity(int image_type_id,
                                                   float intensity) {
  std::lock_guard<std::mutex> lock(update_lock_);
  camera_settings.capture_settings.at(image_type_id)
      .chromatic_aberration_intensity = intensity;
  settings_update_pending = true;
  return true;
}

bool Camera::Impl::SetDepthOfFieldFocalRegion(int image_type_id,
                                              float focal_length) {
  std::lock_guard<std::mutex> lock(update_lock_);
  camera_settings.capture_settings.at(image_type_id)
      .depth_of_field_focal_meters = focal_length;
  settings_update_pending = true;
  return true;
}

bool Camera::Impl::SetDepthOfFieldTransitionRegion(int image_type_id,
                                                   float transition_threshold) {
  std::lock_guard<std::mutex> lock(update_lock_);
  camera_settings.capture_settings.at(image_type_id)
      .depth_of_field_transition_region = transition_threshold;
  settings_update_pending = true;
  return true;
}

bool Camera::Impl::SetFieldOfView(int image_type_id, float field_of_view) {
  std::lock_guard<std::mutex> lock(update_lock_);
  camera_settings.capture_settings.at(image_type_id).fov_degrees =
      field_of_view;
  settings_update_pending = true;
  return true;
}

bool Camera::Impl::SetAperture(float aperture) {
  std::lock_guard<std::mutex> lock(update_lock_);
  camera_settings.aperture = aperture;
  settings_update_pending = true;
  return true;
}

bool Camera::Impl::IsPoseUpdatePending() const { return pose_update_pending; }

bool Camera::Impl::IsSettingsUpdatePending() const {
  return settings_update_pending;
}

const std::string Camera::Impl::GetLookAtObject() const {
  return object_to_look_at;
}

const bool Camera::Impl::GetFrustumEnabled() const { return enable_frustum; }

const int Camera::Impl::GetFrustumCaptureType() const {
  return frustum_capture_type;
}

Pose Camera::Impl::GetRay(ImageType image_type,
                          const Vector2& image_position) const {
  float degrees_horizontal;
  float degrees_vertical;
  Pose pose_ret;

  pose_ret.position.setConstant(std::numeric_limits<float>::quiet_NaN());

  // Calculate direction vector from pixel position
  {
    auto& capture_setting = camera_settings.capture_settings[(int)image_type];
    auto fov_half_degrees = capture_setting.fov_degrees / 2.0f;

    // Note that in NED coodinates, increasing pitch is towards the X-axis (up)
    // and the Y coordinate is given in conventional bitmap coordinates
    //(increasing down) so we need to invert the vertical angle offset
    degrees_horizontal = image_position.x() / capture_setting.width *
                             capture_setting.fov_degrees -
                         fov_half_degrees;
    degrees_vertical = -(image_position.y() / capture_setting.height *
                             capture_setting.fov_degrees -
                         fov_half_degrees);
  }

  // Convert direction vector to global coordinate pose
  auto it = captured_images.find(image_type);

  if (it != captured_images.end()) {
    auto position_data = it->second.GetPositionData();

    pose_ret.position =
        Vector3(position_data[0], position_data[1], position_data[2]);

    // Note that we can't use the Quaternion(float *) constructor which expects
    // (x, y, z, w) order
    pose_ret.orientation =
        Quaternion(position_data[3], position_data[4], position_data[5],
                   position_data[6]) *
        TransformUtils::ToQuaternion(0.0, MathUtils::deg2Rad(degrees_vertical),
                                     MathUtils::deg2Rad(degrees_horizontal));
  }

  return (pose_ret);
}

bool Camera::Impl::HasGimbal() const {
  return camera_settings.gimbal_setting.is_gimbal_mounted;
}

const Transform& Camera::Impl::GetDesiredPose() const {
  return desired_current_pose;
}

const std::string& Camera::Impl::GetGimbalId() const {
  return camera_settings.gimbal_setting.gimbal_id;
}

void Camera::Impl::MarkPoseUpdateAsCompleted() { pose_update_pending = false; }

void Camera::Impl::MarkSettingsUpdateAsCompleted() {
  settings_update_pending = false;
}

const bool Camera::Impl::ResetCameraPose(bool wait_for_pose_update) {
  SetPose(camera_settings.origin_setting, wait_for_pose_update);
  return true;
}

void Camera::Impl::OnBeginUpdate() {
  // Unregister image and info topics
  for (int iimage = 0; iimage < image_entries_.size(); ++iimage) {
    auto& image_entry = image_entries_[iimage];

    // Register topic only if the image-type is capture-enabled
    if (camera_settings.capture_settings[iimage].capture_enabled) {
      topic_manager_.RegisterTopic(
          image_entry.image_topic,
          [this](const Topic& topic, bool is_subscribed) {
            OnSubscribed(topic, is_subscribed);
          });
      topic_manager_.RegisterTopic(
          image_entry.info_topic,
          [this](const Topic& topic, bool is_subscribed) {
            OnSubscribed(topic, is_subscribed);
          });
    }
  }
  topic_manager_.RegisterTopic(onnx_.post_process_image_entry.image_topic,
                               [this](const Topic& topic, bool is_subscribed) {
                                 OnSubscribed_Onnx(topic, is_subscribed);
                               });

  // Publish info topics for each camera type
  for (int ImageTypeCounter = 0;
       ImageTypeCounter < MathUtils::ToNumeric(ImageType::kCount);
       ++ImageTypeCounter) {
    auto capture_setting =
        camera_settings.capture_settings.at(ImageTypeCounter);

    // Only publish info on image types that are capture-enabled
    if (!capture_setting.capture_enabled) continue;

    double focal_length =
        capture_setting.width / 2 /
        tan(TransformUtils::ToRadians(capture_setting.fov_degrees) / 2);

    std::array<double, 5> distortion_parms = {{0, 0, 0, 0, 0}};
    std::array<double, 9> intrinsic_camera_matrix = {
        {focal_length, 0, static_cast<double>(capture_setting.width) / 2, 0,
         focal_length, static_cast<double>(capture_setting.height) / 2, 0, 0,
         1}};
    std::array<double, 9> rectification_matrix = {{1, 0, 0, 0, 1, 0, 0, 0, 1}};
    std::array<double, 12> projection_matrix = {
        {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0}};

    CameraInfoMessage message(capture_setting.width, capture_setting.height, "",
                              distortion_parms, intrinsic_camera_matrix,
                              rectification_matrix, projection_matrix);

    auto iimage_type = capture_setting.image_type;
    Topic* ptopic;

    if ((iimage_type >= 0) && (iimage_type < image_entries_.size()) &&
        !((ptopic = &(image_entries_[iimage_type].info_topic))->IsEmpty())) {
      topic_manager_.PublishTopic(*ptopic, (Message&)message);
    } else {
      logger_.LogError(name_,
                       "[%s] Unsupported ImageType found while publishing "
                       "camera info topics.",
                       id_.c_str());
      throw Error(
          "Unsupported ImageType found while publishing camera info topics.");
    }
  }

  RegisterServiceMethods();
}

bool Camera::Impl::GetCaptureQueueFrontImageTime(TimeNano* timestamp) {
  std::lock_guard<std::mutex> lock(update_lock_);
  if (capture_queue.empty() || timestamp == nullptr) {
    return false;
  } else {
    *timestamp = capture_queue.front();
    return true;
  }
}

bool Camera::Impl::WaitForEarlierCaptures(const TimeNano& timestamp,
                                          int sleep_interval_ns) {
  const auto t_start = std::chrono::steady_clock::now();
  TimeNano queue_front_img_time = 0;  // sets by GetCaptureQueueFrontImageTime()
  while (GetCaptureQueueFrontImageTime(&queue_front_img_time) == false ||
         queue_front_img_time != timestamp) {
    // Check for timeout condition
    auto t_dur = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t_start);
    if (t_dur.count() > kTimeoutPublishSync) {
      logger_.LogError(name_,
                       "[%s] Timeout waiting to publish images in "
                       "capture request order.",
                       id_.c_str());
      return false;
    }

    // Wait for other threads to process the capture queue
    std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_interval_ns));
  }
  return true;
}

void Camera::Impl::PublishImages(std::map<ImageType, ImageMessage>&& images) {
  bool publish_images = false;
  const auto sleep_interval_ns =
      static_cast<int>(camera_settings.capture_interval * 1.0e9f);

  if (!images.empty()) {
    // Wait to publish this set of images until earlier capture requests in the
    // queue have finished publishing
    const TimeNano first_img_timestamp =
        (*images.begin()).second.GetTimestamp();
    if (!WaitForEarlierCaptures(first_img_timestamp, sleep_interval_ns)) {
      return;
    }
    publish_images = true;
  }

  std::lock_guard<std::mutex> lock(update_lock_);
  if (publish_images) {
    // Move the image data to sim camera to save for future GetImages() calls
    captured_images = std::move(images);

    for (const auto& [image_type, image_msg] : captured_images) {
      // Publish captured image
      if (image_type == ImageType::kScene) {
        auto& image_entry = image_entries_[ImageType::kScene];

        if (image_entry.fimage_is_subscribed)
          topic_manager_.PublishTopic(image_entry.image_topic, image_msg);
        if (camera_settings.post_process_model_settings.enabled &&
            onnx_.post_process_image_entry.fimage_is_subscribed &&
            (image_msg.GetEncoding() == "BGR")) {
          auto seg_data = RunOnnxModelOnImages(image_msg);
          auto pos_data = image_msg.GetPositionData();
          std::vector<float> empty_float_data;
          auto pp_img_msg = ImageMessage(
              image_msg.GetTimestamp(), image_msg.GetHeight(),
              image_msg.GetWidth(), image_msg.GetEncoding(),
              image_msg.IsBigEndian(), image_msg.GetStep(), std::move(seg_data),
              std::move(empty_float_data), pos_data[0], pos_data[1], pos_data[2],
              pos_data[3], pos_data[4], pos_data[5], pos_data[6],
              image_msg.GetAnnotations());
          topic_manager_.PublishTopic(
              onnx_.post_process_image_entry.image_topic, pp_img_msg);
        }
      } else {
        ImageEntry* pimage_entry;
        Topic* ptopic;

        if (image_entries_.is_valid_index(image_type) &&
            !((ptopic =
                   &((pimage_entry = &image_entries_[image_type])->image_topic))
                  ->IsEmpty())) {
          topic_manager_.PublishTopic(*ptopic, image_msg);
        } else {
          logger_.LogError(name_,
                           "[%s] Unsupported ImageType found while publishing "
                           "camera image topic.",
                           id_.c_str());
          throw Error(
              "Unsupported ImageType found while publishing camera image "
              "topic.");
        }
      }
    }
  }

  // Finished publishing this image capture request
  capture_queue.pop();
}

void Camera::Impl::AddRequestToCaptureQueue(TimeNano time_stamp) {
  std::lock_guard<std::mutex> lock(update_lock_);
  capture_queue.push(time_stamp);

  // For debug testing queue size limits:
  // logger_.LogVerbose(name_, "[%s] capture_queue size=%d", id_.c_str(),
  //                    capture_queue.size());
}

bool Camera::Impl::IsCaptureQueueFull() const {
  return capture_queue.size() >= kCaptureQueueMaxSize;
}

void Camera::Impl::RegisterServiceMethods() {
  auto set_pose_method_name = topic_path_ + "/" + "SetPose";
  auto set_pose =
      ServiceMethod(set_pose_method_name, {"pose", "wait_for_pose_update"});
  auto set_pose_handler =
      set_pose.CreateMethodHandler(&Camera::Impl::SetPose, *this);
  service_manager_.RegisterMethod(set_pose, set_pose_handler);

  auto get_images_method_name = topic_path_ + "/" + "GetImages";
  auto get_images = ServiceMethod(get_images_method_name, {"image_type_ids"});
  auto get_images_handler =
      get_images.CreateMethodHandler(&Camera::Impl::GetImages, *this);
  service_manager_.RegisterMethod(get_images, get_images_handler);

  auto look_at_object_method_name = topic_path_ + "/" + "LookAtObject";
  auto look_at_object = ServiceMethod(look_at_object_method_name,
                                      {"object_name", "wait_for_pose_update"});
  auto look_at_object_handler =
      look_at_object.CreateMethodHandler(&Camera::Impl::LookAtObject, *this);
  service_manager_.RegisterMethod(look_at_object, look_at_object_handler);

  auto draw_frustum_method_name = topic_path_ + "/" + "DrawFrustum";
  auto draw_frustum =
      ServiceMethod(draw_frustum_method_name, {"image_type", "to_enable"});
  auto draw_frustum_handler =
      draw_frustum.CreateMethodHandler(&Camera::Impl::DrawFrustum, *this);
  service_manager_.RegisterMethod(draw_frustum, draw_frustum_handler);

  auto set_focal_length_method_name = topic_path_ + "/" + "SetFocalLength";
  auto set_focal_length = ServiceMethod(set_focal_length_method_name,
                                        {"image_type_id", "focal_length"});
  auto set_focal_length_handler = set_focal_length.CreateMethodHandler(
      &Camera::Impl::SetFocalLength, *this);
  service_manager_.RegisterMethod(set_focal_length, set_focal_length_handler);

  auto set_field_of_view_method_name = topic_path_ + "/" + "SetFieldOfView";
  auto set_field_of_view = ServiceMethod(set_field_of_view_method_name,
                                         {"image_type_id", "field_of_view"});
  auto set_field_of_view_handler = set_field_of_view.CreateMethodHandler(
      &Camera::Impl::SetFieldOfView, *this);
  service_manager_.RegisterMethod(set_field_of_view, set_field_of_view_handler);

  auto reset_camera_pose_method_name = topic_path_ + "/" + "ResetCameraPose";
  auto reset_camera_pose =
      ServiceMethod(reset_camera_pose_method_name, {"wait_for_pose_update"});
  auto reset_camera_pose_handler = reset_camera_pose.CreateMethodHandler(
      &Camera::Impl::ResetCameraPose, *this);
  service_manager_.RegisterMethod(reset_camera_pose, reset_camera_pose_handler);

  auto set_chromatic_aberration_intensity_name =
      topic_path_ + "/" + "SetChromaticAberrationIntensity";
  auto set_chromatic_aberration = ServiceMethod(
      set_chromatic_aberration_intensity_name, {"image_type_id", "intensity"});
  auto set_chromatic_aberration_handler =
      set_chromatic_aberration.CreateMethodHandler(
          &Camera::Impl::SetChromaticAberrationIntensity, *this);
  service_manager_.RegisterMethod(set_chromatic_aberration,
                                  set_chromatic_aberration_handler);

  auto set_DOF_focal_region_name =
      topic_path_ + "/" + "SetDepthOfFieldFocalRegion";
  auto set_DOF_focal_region = ServiceMethod(set_DOF_focal_region_name,
                                            {"image_type_id", "focal_length"});
  auto set_DOF_focal_region_handler = set_DOF_focal_region.CreateMethodHandler(
      &Camera::Impl::SetDepthOfFieldFocalRegion, *this);
  service_manager_.RegisterMethod(set_DOF_focal_region,
                                  set_DOF_focal_region_handler);

  auto set_DOF_transition_region_name =
      topic_path_ + "/" + "SetDepthOfFieldTransitionRegion";
  auto set_DOF_transition_region =
      ServiceMethod(set_DOF_transition_region_name,
                    {"image_type_id", "transition_threshold"});
  auto set_DOF_transition_region_handler =
      set_DOF_transition_region.CreateMethodHandler(
          &Camera::Impl::SetDepthOfFieldTransitionRegion, *this);
  service_manager_.RegisterMethod(set_DOF_transition_region,
                                  set_DOF_transition_region_handler);

  // TODO: We need to expose extra settings like ISO/shutter speed
  // allongside with aperture. Once we do that and enable manual exposure,
  // we can uncomment these and add the corresponding client API method.
  // auto set_aperture_method_name = topic_path_ + "/" + "SetAperture";
  // auto set_aperture =
  //    ServiceMethod(set_aperture_method_name, {"aperture"});
  // auto set_aperture_handler = set_aperture.CreateMethodHandler(
  //    &Camera::Impl::SetAperture, *this);
  // service_manager_.RegisterMethod(set_aperture, set_aperture_handler);
}

void Camera::Impl::OnEndUpdate() {
  // Unregister image and info topics
  for (auto& image_entry : image_entries_) {
    if (!image_entry.image_topic.IsEmpty() &&
        topic_manager_.ContainsTopic(image_entry.image_topic.GetPath())) {
      topic_manager_.UnregisterTopic(image_entry.image_topic);
    }
    if (!image_entry.info_topic.IsEmpty() &&
        topic_manager_.ContainsTopic(image_entry.image_topic.GetPath())) {
      topic_manager_.UnregisterTopic(image_entry.info_topic);
    }
  }

  if (onnx_.post_process_image_entry.image_topic.IsEmpty() &&
      topic_manager_.ContainsTopic(
          onnx_.post_process_image_entry.image_topic.GetPath())) {
    topic_manager_.UnregisterTopic(onnx_.post_process_image_entry.image_topic);
  }
}

// class camera::loader

Camera::Loader::Loader(Camera::Impl& impl) : impl(impl) {}

void Camera::Loader::Load(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "[%s] Loading 'camera_settings'.",
                          impl.id_.c_str());

  InitializeCaptureSettings();
  LoadCaptureInterval(json);
  LoadCaptureSettings(json);

  InitializeNoiseSettings();
  LoadNoiseSettings(json);

  LoadGimbalSetting(json);
  LoadOriginSetting(json);

  LoadAnnotationSettings(json);
  LoadOnnxModelSettings(json);

  impl.is_loaded_ = true;

  impl.logger_.LogVerbose(impl.name_, "[%s] 'camera_settings' loaded.",
                          impl.id_.c_str());
}

void Camera::Loader::InitializeCaptureSettings() {
  auto image_type_count = MathUtils::ToNumeric(ImageType::kCount);
  impl.camera_settings.capture_settings.resize(image_type_count);
  for (int i = 0; i < image_type_count; ++i) {
    impl.camera_settings.capture_settings.at(i) = CaptureSettings();
    impl.camera_settings.capture_settings.at(i).image_type = i;
    impl.camera_settings.capture_settings.at(i).capture_enabled = false;
  }
  impl.camera_settings.capture_settings.at(static_cast<int>(ImageType::kScene))
      .target_gamma = CaptureSettings::kSceneTargetGamma;
}

void Camera::Loader::LoadCaptureInterval(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "[%s] Loading 'capture_interval'.",
                          impl.id_.c_str());
  impl.camera_settings.capture_interval =
      JsonUtils::GetNumber<float>(json, Constant::Config::capture_interval,
                                  impl.camera_settings.capture_interval);
  impl.logger_.LogVerbose(impl.name_, "[%s] 'capture_interval' loaded.",
                          impl.id_.c_str());
}

void Camera::Loader::LoadCaptureSettings(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "[%s] Loading 'capture_settings'.",
                          impl.id_.c_str());

  auto capture_settings_json =
      JsonUtils::GetArray(json, Constant::Config::capture_settings);
  if (JsonUtils::IsEmptyArray(capture_settings_json)) {
    // Log as warning because camera capture settings should always be
    // provided
    impl.logger_.LogWarning(
        impl.name_, "[%s] 'capture_settings' missing or empty. Using defaults.",
        impl.id_.c_str());
  }

  try {
    std::for_each(capture_settings_json.begin(), capture_settings_json.end(),
                  [this](auto& json) { LoadCaptureSetting(json); });
  } catch (...) {
    impl.camera_settings.capture_settings.clear();
    throw;
  }

  impl.logger_.LogVerbose(impl.name_, "[%s] 'capture_settings' loaded.",
                          impl.id_.c_str());
}

void Camera::Loader::LoadCaptureSetting(const json& json) {
  CaptureSettings setting;

  auto id = JsonUtils::GetInteger(json, Constant::Config::image_type);

  if ((id < 0) || (id >= MathUtils::ToNumeric(ImageType::kCount))) {
    impl.logger_.LogError(
        impl.name_, "[%s] Invalid 'image-type' when loading capture settings.",
        impl.id_.c_str());
    throw Error("Invalid 'image-type' when loading capture settings.");
  }

  setting.image_type = id;
  setting.capture_enabled = JsonUtils::GetBoolean(
      json, Constant::Config::capture_enabled, setting.capture_enabled);
  setting.streaming_enabled = JsonUtils::GetBoolean(
      json, Constant::Config::streaming_enabled, setting.streaming_enabled);
  setting.show_debug_plots = JsonUtils::GetBoolean(
      json, Constant::Config::show_debug_plots, setting.show_debug_plots);
  setting.width =
      JsonUtils::GetInteger(json, Constant::Config::width, setting.width);
  setting.height =
      JsonUtils::GetInteger(json, Constant::Config::height, setting.height);
  setting.fov_degrees = JsonUtils::GetNumber<float>(
      json, Constant::Config::fov_degrees, setting.fov_degrees);
  setting.pixels_as_float = JsonUtils::GetBoolean(
      json, Constant::Config::pixels_as_float, setting.pixels_as_float);
  setting.compress =
      JsonUtils::GetBoolean(json, Constant::Config::compress, setting.compress);

  setting.auto_exposure_method =
      JsonUtils::GetInteger(json, Constant::Config::auto_exposure_method,
                            setting.auto_exposure_method);
  setting.auto_exposure_speed = JsonUtils::GetNumber<float>(
      json, Constant::Config::auto_exposure_speed, setting.auto_exposure_speed);
  setting.auto_exposure_bias = JsonUtils::GetNumber<float>(
      json, Constant::Config::auto_exposure_bias, setting.auto_exposure_bias);
  setting.auto_exposure_max_brightness = JsonUtils::GetNumber<float>(
      json, Constant::Config::auto_exposure_max_brightness,
      setting.auto_exposure_max_brightness);
  setting.auto_exposure_min_brightness = JsonUtils::GetNumber<float>(
      json, Constant::Config::auto_exposure_min_brightness,
      setting.auto_exposure_min_brightness);
  setting.auto_exposure_low_percent = JsonUtils::GetNumber<float>(
      json, Constant::Config::auto_exposure_low_percent,
      setting.auto_exposure_low_percent);
  setting.auto_exposure_high_percent = JsonUtils::GetNumber<float>(
      json, Constant::Config::auto_exposure_high_percent,
      setting.auto_exposure_high_percent);
  setting.auto_exposure_histogram_log_min = JsonUtils::GetNumber<float>(
      json, Constant::Config::auto_exposure_histogram_log_min,
      setting.auto_exposure_histogram_log_min);
  setting.auto_exposure_histogram_log_max = JsonUtils::GetNumber<float>(
      json, Constant::Config::auto_exposure_histogram_log_max,
      setting.auto_exposure_histogram_log_max);
  setting.motion_blur_amount = JsonUtils::GetNumber<float>(
      json, Constant::Config::motion_blur_amount, setting.motion_blur_amount);

  setting.target_gamma = JsonUtils::GetNumber<float>(
      json, Constant::Config::target_gamma, setting.target_gamma);

  setting.max_depth_meters = JsonUtils::GetNumber(
      json, Constant::Config::max_depth_meters, setting.max_depth_meters);

  setting.depth_of_field_focal_meters =
      JsonUtils::GetNumber(json, Constant::Config::depth_of_field_focal_region,
                           setting.depth_of_field_focal_meters);
  setting.depth_of_field_transition_region = JsonUtils::GetNumber(
      json, Constant::Config::depth_of_field_transition_region,
      setting.depth_of_field_transition_region);
  setting.chromatic_aberration_intensity = JsonUtils::GetNumber(
      json, Constant::Config::chromatic_aberration_intensity,
      setting.chromatic_aberration_intensity);
  // TODO: add other settings

  impl.camera_settings.capture_settings.at(id) = setting;
}

void Camera::Loader::InitializeNoiseSettings() {
  auto image_type_count = MathUtils::ToNumeric(ImageType::kCount);
  impl.camera_settings.noise_settings.resize(image_type_count);
  for (int i = 0; i < image_type_count; ++i) {
    impl.camera_settings.noise_settings.at(i) = NoiseSettings();
  }
}

void Camera::Loader::LoadNoiseSettings(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "[%s] Loading 'noise_settings'.",
                          impl.id_.c_str());

  auto noise_settings_json =
      JsonUtils::GetArray(json, Constant::Config::noise_settings);
  if (JsonUtils::IsEmptyArray(noise_settings_json)) {
    impl.logger_.LogVerbose(
        impl.name_, "[%s] 'noise_settings' missing or empty. Using defaults.",
        impl.id_.c_str());
  }

  try {
    std::for_each(noise_settings_json.begin(), noise_settings_json.end(),
                  [this](auto& json) { LoadNoiseSetting(json); });
  } catch (...) {
    impl.camera_settings.noise_settings.clear();
    throw;
  }

  impl.logger_.LogVerbose(impl.name_, "[%s] 'noise_settings' loaded.",
                          impl.id_.c_str());
}

void Camera::Loader::LoadNoiseSetting(const json& json) {
  NoiseSettings setting;

  auto id = JsonUtils::GetInteger(json, Constant::Config::image_type);

  if ((id < 0) || (id >= MathUtils::ToNumeric(ImageType::kCount))) {
    impl.logger_.LogError(
        impl.name_, "[%s] Invalid 'image-type' when loading noise settings.",
        impl.id_.c_str());
    throw Error("Invalid 'image-type' when loading noise settings.");
  }

  setting.Enabled =
      JsonUtils::GetBoolean(json, Constant::Config::enabled, setting.Enabled);

  setting.RandSpeed = JsonUtils::GetNumber<float>(
      json, Constant::Config::rand_speed, setting.RandSpeed);
  setting.RandSize = JsonUtils::GetNumber<float>(
      json, Constant::Config::rand_size, setting.RandSize);
  setting.RandDensity = JsonUtils::GetNumber<float>(
      json, Constant::Config::rand_density, setting.RandDensity);
  setting.RandContrib = JsonUtils::GetNumber<float>(
      json, Constant::Config::rand_contrib, setting.RandContrib);

  setting.HorzWaveContrib = JsonUtils::GetNumber<float>(
      json, Constant::Config::horz_wave_contrib, setting.HorzWaveContrib);
  setting.HorzWaveStrength = JsonUtils::GetNumber<float>(
      json, Constant::Config::horz_wave_strength, setting.HorzWaveStrength);
  setting.HorzWaveVertSize = JsonUtils::GetNumber<float>(
      json, Constant::Config::horz_wave_vert_size, setting.HorzWaveVertSize);
  setting.HorzWaveScreenSize =
      JsonUtils::GetNumber<float>(json, Constant::Config::horz_wave_screen_size,
                                  setting.HorzWaveScreenSize);

  setting.HorzNoiseLinesContrib = JsonUtils::GetNumber<float>(
      json, Constant::Config::horz_noise_lines_contrib,
      setting.HorzNoiseLinesContrib);
  setting.HorzNoiseLinesDensityY = JsonUtils::GetNumber<float>(
      json, Constant::Config::horz_noise_lines_density_y,
      setting.HorzNoiseLinesDensityY);
  setting.HorzNoiseLinesDensityXY = JsonUtils::GetNumber<float>(
      json, Constant::Config::horz_noise_lines_density_xy,
      setting.HorzNoiseLinesDensityXY);

  setting.HorzDistortionContrib = JsonUtils::GetNumber<float>(
      json, Constant::Config::horz_distortion_contrib,
      setting.HorzDistortionContrib);
  setting.HorzDistortionStrength = JsonUtils::GetNumber<float>(
      json, Constant::Config::horz_distortion_strength,
      setting.HorzDistortionStrength);

  impl.camera_settings.noise_settings.at(id) = setting;
}

void Camera::Loader::LoadOriginSetting(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "Loading 'origin'.");

  auto origin_json = JsonUtils::GetJsonObject(json, Constant::Config::origin);
  if (JsonUtils::IsEmpty(origin_json)) {
    impl.logger_.LogVerbose(impl.name_,
                            "'origin' missing or empty. Using default.");
  } else {
    impl.camera_settings.origin_setting =
        JsonUtils::GetTransform(json, Constant::Config::origin);
    impl.desired_current_pose =
        JsonUtils::GetTransform(json, Constant::Config::origin);
  }
  impl.logger_.LogVerbose(impl.name_, "'origin' loaded.");
}

void Camera::Loader::LoadGimbalSetting(const json& json) {
  impl.logger_.LogVerbose(impl.name_,
                          "[%s] Loading 'Gimbal Settings' for camera.",
                          impl.id_.c_str());

  auto gimbal_json = JsonUtils::GetJsonObject(json, Constant::Config::gimbal);
  if (JsonUtils::IsEmpty(gimbal_json)) {
    impl.logger_.LogVerbose(
        impl.name_, "[%s] 'gimbal settings' missing or empty. Using default.",
        impl.id_.c_str());
  } else {
    impl.camera_settings.gimbal_setting.gimbal_id =
        JsonUtils::GetString(gimbal_json, Constant::Config::gimbal_id,
                             impl.camera_settings.gimbal_setting.gimbal_id);
    if (!impl.camera_settings.gimbal_setting.gimbal_id.empty()) {
      impl.logger_.LogVerbose(impl.name_, "[%s] New 'gimbal' configured.",
                              impl.id_.c_str());
      impl.camera_settings.gimbal_setting.is_gimbal_mounted = true;
    }

    impl.camera_settings.gimbal_setting.lock_roll =
        JsonUtils::GetBoolean(gimbal_json, Constant::Config::lock_roll,
                              impl.camera_settings.gimbal_setting.lock_roll);
    impl.camera_settings.gimbal_setting.lock_pitch =
        JsonUtils::GetBoolean(gimbal_json, Constant::Config::lock_pitch,
                              impl.camera_settings.gimbal_setting.lock_pitch);
    impl.camera_settings.gimbal_setting.lock_yaw =
        JsonUtils::GetBoolean(gimbal_json, Constant::Config::lock_yaw,
                              impl.camera_settings.gimbal_setting.lock_yaw);
  }
  impl.logger_.LogVerbose(impl.name_, "'gimbal settings' loaded for camera.");
}

void LoadBBoxSettings(const json& bbox_json, BBoxSettings& bbox_settings) {
  bbox_settings.alignment =
      bbox_json.value(Constant::Config::alignment, bbox_settings.alignment);
}

void Camera::Loader::LoadAnnotationSettings(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "Loading 'annotation-settings'.");

  auto annotation_json =
      JsonUtils::GetJsonObject(json, Constant::Config::annotation_settings);
  if (JsonUtils::IsEmpty(annotation_json)) {
    impl.logger_.LogVerbose(
        impl.name_, "'annotation-settings' missing or empty. Using default.");
  } else {
    auto object_ids_json = JsonUtils::GetArray(
        annotation_json, Constant::Config::object_ids,
        impl.camera_settings.annotation_settings.object_ids);

    if (JsonUtils::IsEmptyArray(object_ids_json)) {
      impl.logger_.LogWarning(impl.name_,
                              "[%s] 'bbox2d-object-ids' missing or empty.",
                              impl.id_.c_str());
      return;
    }

    impl.camera_settings.annotation_settings.object_ids.insert(
        object_ids_json.begin(), object_ids_json.end());

    impl.camera_settings.annotation_settings.enabled =
        JsonUtils::GetBoolean(annotation_json, Constant::Config::enabled,
                              impl.camera_settings.annotation_settings.enabled);

    auto bbox2D_settings = JsonUtils::GetJsonObject(
        annotation_json, Constant::Config::bbox2D_settings);
    LoadBBoxSettings(bbox2D_settings,
                     impl.camera_settings.annotation_settings.bbox2D_settings);

    auto bbox3D_settings = JsonUtils::GetJsonObject(
        annotation_json, Constant::Config::bbox3D_settings);
    LoadBBoxSettings(bbox3D_settings,
                     impl.camera_settings.annotation_settings.bbox3D_settings);

    impl.camera_settings.annotation_settings.distance_filter_enabled =
        JsonUtils::GetBoolean(
            annotation_json, Constant::Config::distance_filter_enabled,
            impl.camera_settings.annotation_settings.distance_filter_enabled);

    impl.camera_settings.annotation_settings.distance_filter_range =
        JsonUtils::GetNumber(
            annotation_json, Constant::Config::distance_filter_range,
            impl.camera_settings.annotation_settings.distance_filter_range);
  }
  impl.logger_.LogVerbose(impl.name_, "'annotation-settings' loaded.");
}

void Camera::Loader::LoadOnnxModelSettings(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "Loading 'post-process-onnx-settings'.");

  auto onnx_json = JsonUtils::GetJsonObject(
      json, Constant::Config::post_process_model_settings);
  if (JsonUtils::IsEmpty(onnx_json)) {
    impl.logger_.LogVerbose(impl.name_,
                            "'post-process-onnx-settings' missing or empty. "
                            "Disabling post process onnx step.");
    impl.camera_settings.post_process_model_settings.enabled = false;
  } else {
    impl.camera_settings.post_process_model_settings.enabled =
        JsonUtils::GetBoolean(onnx_json, Constant::Config::enabled, false);
    impl.camera_settings.post_process_model_settings.filepath =
        JsonUtils::GetString(onnx_json, Constant::Config::model_filepath, "");
    if (!impl.camera_settings.post_process_model_settings.enabled ||
        impl.camera_settings.post_process_model_settings.filepath.empty()) {
      impl.camera_settings.post_process_model_settings.enabled = false;
      return;
    }
    impl.camera_settings.post_process_model_settings.execution_provider =
        JsonUtils::GetString(onnx_json, Constant::Config::execution_provider,
                             "cpu");
  }
  impl.logger_.LogVerbose(impl.name_, "Loaded 'post-process-onnx-settings'.");
}

}  // namespace projectairsim
}  // namespace microsoft
