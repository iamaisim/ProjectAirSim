// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/transforms/transform_tree.hpp"

#include <stack>

#include "Eigen/Dense"
#include "Eigen/Geometry"

#ifdef _WIN32
extern "C" void OutputDebugStringA(const char*);
#else
#include <stdio.h>
#endif  //_WIN32

namespace microsoft {
namespace projectairsim {

const TransformTree::ConstRefFrame TransformTree::kRefFrameGlobal(
    "GLOBAL");  // Predefined reference frame object
                // representing global coordinates
const TransformTree::RefFrameID TransformTree::kRefFrameIDGlobal =
    TransformTree::ToRefFrameID(
        &kRefFrameGlobal);  // ID of the global reference frame

TransformTree::TransformTree(void) noexcept
    : fdo_cached(false), map_ref_frame_id_vecref_frame_id_() {
  // Add global reference frame manually since it is shared and should not have
  // a pointer to the TransformTree
  map_ref_frame_id_vecref_frame_id_.emplace(kRefFrameIDGlobal, RefFrameID());
}

TransformTree::~TransformTree() {
  // We're going away--remove pointer to ourselves from all registered RefFrames
  std::lock_guard<std::mutex> lg(mutex_map_);

  for (auto pair : map_ref_frame_id_vecref_frame_id_)
    ToRefFrame(pair.first)->ptransformtree_parent_ = nullptr;
}

void TransformTree::CombinePose(Pose* ppose_inout, const Pose& pose2) {
  // Rotate the position vector by the orientation of the frame
  auto rotated_position = pose2.orientation * ppose_inout->position;
  // Translate the position to the position of frame to get the broader position
  ppose_inout->position = pose2.position + rotated_position;
  ppose_inout->orientation = ppose_inout->orientation * pose2.orientation;
}

void TransformTree::CombinePoseInverse(Pose* ppose_inout, const Pose& pose2) {
  // Substract the position of the frame to the position vector to get a rotated
  // relative position
  ppose_inout->position -= pose2.position;
  // Rotate the position vector by the inverse orientation of the frame to get
  // the relative position
  ppose_inout->position = pose2.orientation.inverse() * ppose_inout->position;

  ppose_inout->orientation =
      ppose_inout->orientation * pose2.orientation.inverse();
}

bool TransformTree::Convert(const Pose& poseFrom, const RefFrame& ref_frameFrom,
                            const RefFrame& ref_frameTo, Pose* pposeTo) const {
  bool fok_ret = true;
  auto ref_frame_id_from = ToRefFrameID(&ref_frameFrom);
  auto ref_frame_id_to = ToRefFrameID(&ref_frameTo);
  const RefFrameID* pref_frame_id_from_first = nullptr;
  const RefFrameID* pref_frame_id_from_last = nullptr;
  const RefFrameID* pref_frame_id_to_first = nullptr;
  const RefFrameID* pref_frame_id_to_last = nullptr;

  if ((ref_frame_id_from != kRefFrameIDGlobal) &&
      (ref_frameFrom.ptransformtree_parent_ == nullptr))
    throw std::logic_error(
        "TransformTree::Convert(): 'From' RefFrame is not registered");
  if ((ref_frame_id_to != kRefFrameIDGlobal) &&
      (ref_frameTo.ptransformtree_parent_ == nullptr))
    throw std::logic_error(
        "TransformTree::Convert(): 'To' RefFrame is not registered");

  std::lock_guard<std::mutex> lg(*const_cast<std::mutex*>(&mutex_map_));

  // Get root frames and ancestor frame chains
  {
    auto it = map_ref_frame_id_vecref_frame_id_.find(ref_frame_id_from);
    const VecRefFrameID* pvecref_frame_id_from = nullptr;
    const VecRefFrameID* pvecref_frame_id_to = nullptr;
    RefFrameID ref_frame_id_from_root, ref_frame_id_to_root;

    if (it == map_ref_frame_id_vecref_frame_id_.end())
      throw std::logic_error(
          "TransformTree::Convert(): 'From' RefFrame is not in the map");
    pvecref_frame_id_from = &it->second;

    it = map_ref_frame_id_vecref_frame_id_.find(ref_frame_id_to);
    if (it == map_ref_frame_id_vecref_frame_id_.end())
      throw std::logic_error(
          "TransformTree::Convert(): 'To' RefFrame is not in the map");
    pvecref_frame_id_to = &it->second;

    // Get pointers into the parent chains
    if (pvecref_frame_id_from->empty())
      ref_frame_id_from_root = ref_frame_id_from;
    else {
      pref_frame_id_from_first = &pvecref_frame_id_from->front();
      pref_frame_id_from_last = &pvecref_frame_id_from->back();
      ref_frame_id_from_root = *pref_frame_id_from_last;
    }
    if (pvecref_frame_id_to->empty())
      ref_frame_id_to_root = ref_frame_id_to;
    else {
      pref_frame_id_to_first = &pvecref_frame_id_to->front();
      pref_frame_id_to_last = &pvecref_frame_id_to->back();
      ref_frame_id_to_root = *pref_frame_id_to_last;
    }

    // Verify they have the same root reference frame or the RefFrames don't
    // have a common parent and it's impossible to map from one to the other
    if (ref_frame_id_from_root != ref_frame_id_to_root) {
      fok_ret = false;
      goto LError;
    }
  }

  // If we're in cached mode, fill the refFrame caches
  if (fdo_cached) {
    // Ensure the caches are set for both parent chains
    EnsureParentChainCache(ref_frame_id_from, pref_frame_id_from_first,
                           pref_frame_id_from_last);
    EnsureParentChainCache(ref_frame_id_to, pref_frame_id_to_first,
                           pref_frame_id_to_last);

    // Map the pose from the "from" RefFrame to the global frame
    *pposeTo = poseFrom;
    CombinePose(pposeTo, ref_frameFrom.GetGlobalPoseCache().Get());

    // Map the pose from the global frame to the "to" RefFrame
    CombinePose(pposeTo, ref_frameTo.GetGlobalPoseCache().GetInverse());

  } else {
    // Follow the parent chains to convert the pose
    *pposeTo = poseFrom;
    MapPose(pposeTo, ref_frame_id_from, pref_frame_id_from_first,
            pref_frame_id_from_last, ref_frame_id_to, pref_frame_id_to_first,
            pref_frame_id_to_last);
  }

LError:
  return (fok_ret);
}

void TransformTree::Dump(void) {
  struct Frame {
    bool fis_last;
    std::string str_prefix;
    RefFrameID ref_frame_id;

    Frame(RefFrameID ref_frame_idIn, const std::string& str_prefixIn,
          bool fis_lastIn = false)
        : fis_last(fis_lastIn),
          ref_frame_id(ref_frame_idIn),
          str_prefix(str_prefixIn) {}
  };
  std::stack<Frame> stack_frame;

#ifdef _WIN32
  auto PrintDebug = [](const char* sz) { OutputDebugStringA(sz); };
#else
  auto PrintDebug = [](const char* sz) { fputs(sz, stdout); };
#endif  //_WIN32

  PrintDebug(
      "\"frame_name\" ref_frame_addr (pos_x, pos_y, pos_q)q(quat_x, quat_y, "
      "quat_z, quat_w)");
  stack_frame.push(Frame(kRefFrameIDGlobal, "", true));

  while (!stack_frame.empty()) {
    Frame frame = stack_frame.top();
    auto& ref_frame = *ToRefFrame(frame.ref_frame_id);
    std::string str_out = frame.str_prefix;
    char sz[256];
    auto pose = ref_frame.GetLocalPose();

    stack_frame.pop();
    str_out.append(1, '\"');
    str_out.append(ref_frame.GetID());
    sprintf(sz, "\" %#tx (%f, %f, %f)q(%f, %f, %f, %f)\r\n", frame.ref_frame_id,
            pose.position.x(), pose.position.y(), pose.position.z(),
            pose.orientation.x(), pose.orientation.y(), pose.orientation.z(),
            pose.orientation.w());

    str_out.append(sz);
    PrintDebug(str_out.c_str());

    // Add children to stack
    {
      std::string str_prefixChild;
      Frame* pframeLast = nullptr;
      bool fIsFirst = true;

      if (frame.str_prefix.empty())
        str_prefixChild = '+';
      else {
        str_prefixChild =
            frame.str_prefix.substr(0, frame.str_prefix.size() - 1) +
            (frame.fis_last ? " +" : "|+");
      }

      for (auto pair : map_ref_frame_id_vecref_frame_id_) {
        auto& vecref_frame_id = pair.second;

        if (!vecref_frame_id.empty() &&
            (vecref_frame_id[0] == frame.ref_frame_id)) {
          pframeLast = &stack_frame.emplace(
              Frame(pair.first, str_prefixChild, fIsFirst));
          fIsFirst = false;  // The first child will be the last printed since
                             // we're using a stack
        }
      }
    }
  }
}

void TransformTree::EnsureParentChainCache(
    RefFrameID ref_frame_id, const RefFrameID* pref_frame_id_ancestor_first,
    const RefFrameID* pref_frame_id_ancestor_last) {
  const RefFrameID* pref_frame_id;
  Pose pose;

  if (ToRefFrame(ref_frame_id)->GetGlobalPoseCache().FIsValid())
    return;  // If the leaf's cache is already set, all of ancestor caches are
             // set too

  if (pref_frame_id_ancestor_first != nullptr) {
    const Pose* pposeLast = nullptr;

    if (*pref_frame_id_ancestor_last != kRefFrameIDGlobal)
      throw std::logic_error(
          "TransformTree::EnsureParentChainCache(): Root frame of the parent "
          "RefFrame chain is not the global frame");

    // Skip RefFrames that already have their cache set and get the last cached
    // global pose
    pref_frame_id = pref_frame_id_ancestor_last - 1;
    for (; pref_frame_id >= pref_frame_id_ancestor_first; --pref_frame_id) {
      auto pref_frame = ToRefFrame(*pref_frame_id);
      auto posecache = pref_frame->GetGlobalPoseCache();

      if (!posecache.FIsValid()) break;

      pposeLast = &posecache.Get();
    }
    if (pposeLast != nullptr) pose = *pposeLast;

    // Calculate global poses for ancestor frames and cache them
    for (; pref_frame_id >= pref_frame_id_ancestor_first; --pref_frame_id) {
      auto pref_frame = ToRefFrame(*pref_frame_id);

      CombinePose(&pose, pref_frame->GetLocalPose());
      pref_frame->GetGlobalPoseCache().Set(pose);
    }
  }

  // Calculate global pose for the leaf frame and cache it
  {
    auto pref_frame = ToRefFrame(ref_frame_id);

    CombinePose(&pose, pref_frame->GetLocalPose());
    pref_frame->GetGlobalPoseCache().Set(pose);
  }
}

void TransformTree::MapPose(Pose* ppose_inout, RefFrameID ref_frame_id_from,
                            const RefFrameID* pref_frame_id_from_first,
                            const RefFrameID* pref_frame_id_from_last,
                            RefFrameID ref_frame_id_to,
                            const RefFrameID* pref_frame_id_to_first,
                            const RefFrameID* pref_frame_id_to_last) {
  // Starting from the root, find the first pair of parent RefFrameID's
  // that don't match; pref_frameFromLast and preffreamToLast will then
  // point to the Nearest (leaf-most) Common Parent (NCP).
  if ((pref_frame_id_from_last != nullptr) &&
      (pref_frame_id_to_last != nullptr)) {
    for (auto *pref_frame_id_from = pref_frame_id_from_last - 1,
              *pref_frame_id_to = pref_frame_id_to_last - 1;
         (pref_frame_id_from >= pref_frame_id_from_first) &&
         (pref_frame_id_to >= pref_frame_id_from_first);
         --pref_frame_id_from, --pref_frame_id_to) {
      if (*pref_frame_id_from != *pref_frame_id_to) break;

      pref_frame_id_from_last = pref_frame_id_from;
      pref_frame_id_to_last = pref_frame_id_to;
    }
  }

  // Convert the pose from the "from" RefFrame up to the NCP
  MapPoseFromRefFrame(ref_frame_id_from, ppose_inout);
  if (pref_frame_id_from_last != nullptr) {
    for (auto pref_frame_id = pref_frame_id_from_first;
         pref_frame_id < pref_frame_id_from_last; ++pref_frame_id) {
      MapPoseFromRefFrame(*pref_frame_id, ppose_inout);
    }
  }

  // Convert the pose from the NCP down to the "to" RefFrame
  if (pref_frame_id_to_last != nullptr) {
    for (auto pref_frame_id = pref_frame_id_to_last - 1;
         pref_frame_id >= pref_frame_id_to_first; --pref_frame_id) {
      MapPoseToRefFrame(*pref_frame_id, ppose_inout);
    }
  }
  MapPoseToRefFrame(ref_frame_id_to, ppose_inout);
}

void TransformTree::MapPoseFromRefFrame(RefFrameID ref_frame_id_from,
                                        Pose* ppose_inout) {
  auto pose_frame = ToRefFrame(ref_frame_id_from)->GetLocalPose();

  CombinePose(ppose_inout, pose_frame);
}

void TransformTree::MapPoseToRefFrame(RefFrameID ref_frame_id_to,
                                      Pose* ppose_inout) {
  auto pose_frame = ToRefFrame(ref_frame_id_to)->GetLocalPose();

  CombinePoseInverse(ppose_inout, pose_frame);
}

void TransformTree::Register(RefFrame* pref_frame_inout,
                             const RefFrame& ref_frameParent) {
  bool fok_ret = false;
  auto ref_frame_id_parent = ToRefFrameID(&ref_frameParent);

  if (pref_frame_inout->ptransformtree_parent_ != nullptr)
    throw std::logic_error("RefFrame has already been registered");
  if ((ref_frame_id_parent != kRefFrameIDGlobal) &&
      (ref_frameParent.ptransformtree_parent_ == nullptr))
    throw std::logic_error("Parent RefFrame is not registered");

  // Add new entry for ref_frame
  {
    VecRefFrameID vecref_frame_id;
    std::lock_guard<std::mutex> lg(mutex_map_);
    auto it =
        map_ref_frame_id_vecref_frame_id_.find(ToRefFrameID(&ref_frameParent));

    if (it == map_ref_frame_id_vecref_frame_id_.end())
      throw std::logic_error("Parent RefFrame is not in map");

    // Construct parent chain
    vecref_frame_id.push_back(
        ToRefFrameID(&ref_frameParent));  // Add immediate parent
    if (!it->second.empty())              // Append parent's parent chain
    {
      vecref_frame_id.reserve(it->second.size() + 1);
      for (auto ref_frame_idT : it->second)
        vecref_frame_id.push_back(ref_frame_idT);
    }

    map_ref_frame_id_vecref_frame_id_.emplace(ToRefFrameID(pref_frame_inout),
                                              vecref_frame_id);
  }

  // Update RefFrame with the transform tree
  pref_frame_inout->ptransformtree_parent_ = this;
}

void TransformTree::Register(RefFrame& ref_frame_inout,
                             const RefFrame& ref_frameParent) {
  Register(&ref_frame_inout, ref_frameParent);
}

void TransformTree::StartCachedMode(void) {
  // Invalidate caches on all registered RefFrames
  for (auto pair : map_ref_frame_id_vecref_frame_id_)
    ToRefFrame(pair.first)->GetGlobalPoseCache().Invalidate();

  // Enable using cached poses
  fdo_cached = true;
}

void TransformTree::TransferRegistration(RefFrame* pref_frame_old,
                                         RefFrame* pref_frame_new) {
  std::lock_guard<std::mutex> lg(mutex_map_);
  auto ref_frame_id_new = ToRefFrameID(pref_frame_new);
  auto ref_frame_id_old = ToRefFrameID(pref_frame_old);

  // Change the RefFrameID entry for the old RefFrame
  {
    auto it = map_ref_frame_id_vecref_frame_id_.find(ref_frame_id_old);

    if (it == map_ref_frame_id_vecref_frame_id_.end())
      throw std::logic_error(
          "Can't transfer RefFrame registration--old RefFrame is not in the "
          "map");

    map_ref_frame_id_vecref_frame_id_.emplace(
        ref_frame_id_new,
        it->second);  // Add new entry with existing parent chain
    map_ref_frame_id_vecref_frame_id_.erase(it);  // Remove old entry
  }

  // Update RefFrameID in all parent chains
  for (auto& pair : map_ref_frame_id_vecref_frame_id_) {
    auto& vecref_frame_id = pair.second;

    // Locate matching RefFrameID in entry's parent chain
    for (auto *pref_frame_idFirst = &vecref_frame_id[0],
              *pref_frame_id = pref_frame_idFirst,
              *pref_frame_idMax = pref_frame_id + vecref_frame_id.size();
         pref_frame_id < pref_frame_idMax; ++pref_frame_id) {
      if (*pref_frame_id == ref_frame_id_old) {
        // Swap old ID for new
        *pref_frame_id = ref_frame_id_new;
        break;
      }
    }
  }

  // New RefFrame is now registered and the old refFrame is no longer
  // registered
  pref_frame_new->ptransformtree_parent_ = this;
  pref_frame_old->ptransformtree_parent_ = nullptr;
}

void TransformTree::Unregister(RefFrame* pref_frame) {
  std::lock_guard<std::mutex> lg(mutex_map_);
  auto ref_frame_idRemove = ToRefFrameID(pref_frame);

  // Remove entry for this ref_frame
  {
    auto it = map_ref_frame_id_vecref_frame_id_.find(ref_frame_idRemove);

    assert(it != map_ref_frame_id_vecref_frame_id_
                     .end());  // The RefFrame must always be in the map if it
                               // has a valid TransformTree pointer
    if (it != map_ref_frame_id_vecref_frame_id_.end())
      map_ref_frame_id_vecref_frame_id_.erase(it);
  }

  // Remove from all parent chains
  for (auto& pair : map_ref_frame_id_vecref_frame_id_) {
    auto& vecref_frame_id = pair.second;

    // Locate matching RefFrameID in entry's parent chain
    for (auto *pref_frame_idFirst = &vecref_frame_id[0],
              *pref_frame_id = pref_frame_idFirst,
              *pref_frame_idMax = pref_frame_id + vecref_frame_id.size();
         pref_frame_id < pref_frame_idMax; ++pref_frame_id) {
      if (*pref_frame_id == ref_frame_idRemove) {
        // vecref_frame_id parent chain is in leaf (immediate parent) to root
        // (global) order; truncate parent chain at this entry, orphaning this
        // RefFrame
        vecref_frame_id.resize(pref_frame_id - pref_frame_idFirst);
        break;
      }
    }
  }

  // RefFrame is no longer registered
  pref_frame->ptransformtree_parent_ = nullptr;
}

void TransformTree::Unregister(RefFrame& ref_frame) { Unregister(&ref_frame); }

TransformTree::RefFrame::RefFrame(const std::string& id) noexcept
    : id_(id),
      ppose_cache_global_(std::make_unique<PoseCache>()),
      ptransformtree_parent_(nullptr) {}

TransformTree::RefFrame::RefFrame(RefFrame&& ref_frame_other) noexcept
    : ptransformtree_parent_(ref_frame_other.ptransformtree_parent_),
      ppose_cache_global_(std::move(ref_frame_other.ppose_cache_global_)) {
  ref_frame_other.ptransformtree_parent_ = nullptr;
  ref_frame_other.ppose_cache_global_ = nullptr;

  if (ptransformtree_parent_ != nullptr)
    ptransformtree_parent_->TransferRegistration(&ref_frame_other, this);
}

TransformTree::RefFrame::~RefFrame(void) {
  if (ptransformtree_parent_ != nullptr)
    ptransformtree_parent_->Unregister(this);
}

TransformTree::RefFrame& TransformTree::RefFrame::operator=(
    RefFrame&& ref_frame_other) noexcept {
  ptransformtree_parent_ = ref_frame_other.ptransformtree_parent_;
  ref_frame_other.ptransformtree_parent_ = nullptr;

  if (ptransformtree_parent_ != nullptr)
    ptransformtree_parent_->TransferRegistration(&ref_frame_other, this);

  return (*this);
}

TransformTree::ConstRefFrame& TransformTree::ConstRefFrame::operator=(
    ConstRefFrame&& const_ref_frame_other) noexcept {
  RefFrame::operator=(std::move(const_ref_frame_other));
  pose_ = const_ref_frame_other.pose_;
  return (*this);
}

TransformTree::IndirectRefFrame& TransformTree::IndirectRefFrame::operator=(
    IndirectRefFrame&& indirect_ref_frame_other) noexcept {
  RefFrame::operator=(std::move(indirect_ref_frame_other));
  ppose_ = indirect_ref_frame_other.ppose_;

  return (*this);
}

TransformTree::StaticRefFrame& TransformTree::StaticRefFrame::operator=(
    StaticRefFrame&& static_ref_frame_other) noexcept {
  RefFrame::operator=(std::move(static_ref_frame_other));
  pose_ = static_ref_frame_other.pose_;

  return (*this);
}

void TransformTree::RefFrame::PoseCache::Set(const Pose& pose) {
  pose_ = pose;
  pose_inverse_.orientation = pose.orientation.inverse();
  pose_inverse_.position = -pose.position;

  fposes_are_valid_ = true;
}

}  // namespace projectairsim
}  // namespace microsoft
