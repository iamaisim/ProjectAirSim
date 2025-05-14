// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_TRANSFORMS_TRANSFORM_TREE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_TRANSFORMS_TRANSFORM_TREE_HPP_

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "core_sim/physics_common_types.hpp"
#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

//----------------------------------------------------------------------------
// Class TransformTree
//
// This class manages a tree hierarchy of reference frames and provides
// conversion of poses (position and orientation) between reference frames.
//
// Reference frames are subclasses of the RefFrame class and specifies the
// reference frame's pose with respect to its parent reference frame.
// RefFrame objects are added to the tree via the Register() method which
// also specifies the parent reference frame.  All reference frames (except the
// global reference frame, kRefFrameGlobal) have a parent reference frame--
// the parent-child pairs form a tree with the root being the global
// reference frame.
//
// Typically reference frames will be instances of IndirectRefFrame or
// StaticRefFrame which return the frame's pose via pointer and from a copy
// respectively, but feel free to create custom subclasses of RefFrame as
// the situation dictates.
//
// Reference frames are arbitrary and not necessarily tied to a specific
// actor's current pose.  For instance, each Robot class instance has
// two associated reference frames: one for the robot's current pose and another
// for the robot's home pose.  The current pose reference frame is an
// IndirectRefFrame pointing to the robot's kinematics object to avoid copying
// the pose into a RefFrame whenever the robot moves.  On the other hand, the
// home pose is a StaticRefFrame instance (since that changes infrequently if at
// all) and is completely independent.
//
// For convenience, classes like Robot have a conversion operator to RefFrame
// that returns the current pose reference frame.  This allows a class instance
// to be passed directly to TransformTree methods like Convert() to make
// converting a pose between robot reference frames more natural, like in this
// call to convert a pose in the robot's reference frame to the global
// reference frame:
//
//      ptransformtree->Convert(poseInVehicle1, robotVehicle1,
//              TransformTree::kRefFrameGlobal, &poseInGlobal);
//
// or this call to convert a pose between two robots by simply specifying the
// two robots as the reference frames:
//
//      ptransformtree->Convert(poseInVehicle1, robotVehicle1, robotVehicle2,
//              &poseInVehicle2);
//
// In contrast, the robot's home reference frame must be passed explicitly:
//
//      ptransformtree->Convert(poseInHome, robotVehicle1.GetHomeRefFrame(),
//              TransformTree::kRefFrameGlobal, &poseInGlobal);
//
// Cached mode is enabled or restarted by StartCachedMode() and disabled by
// StopCachedMode().  Cached mode caches the global pose of each RefFrame
// involved in a pose conversion the first time each RefFrame is referenced.
// Subsequent references use the cached global pose for much more efficient
// pose conversions.  When the TransformTree is always used in cached mode,
// such in the fast physics engine, StartCachedMode() is called at the start
// of each update pass to invalidate the cache and StopCachedMode() doesn't
// need to ever be called.
//
// Multithreading Safety:  TransformTree methods are multithread-safe, but they
// call methods in registered RefFrames which TransformTree cannot guarentee are
// multithread-safe.
//
class TransformTree {
  // Protected Types, Forward Declarations
 protected:
  typedef std::intptr_t RefFrameID;  // ID of a reference frame

  // Public Types
 public:
  //--------------------------------------------------------------------------
  // Class RefFrame
  //
  // Base class representing a coordinate reference frame.  Almost all
  // instances will be of a subclass, but this class can be instantiated
  // directly for a reference frame that always has the identity pose.
  //
  // A reference frame need only specify the pose of the reference frame with
  // respect to it's parent.
  //
  // RefFrame's can only be moved, not copied.  The class works with
  // TransformTree to automatically handle the move or destruction of a
  // registered RefFrame.
  //--------------------------------------------------------------------------
  class RefFrame {
    // Public Methods
   public:
    RefFrame(const std::string& id) noexcept;
    RefFrame(RefFrame&) = delete;  // Disallow copy
    RefFrame(RefFrame&& ref_frame_other) noexcept;
    virtual ~RefFrame(void);

    // Return the reference frame's ID
    virtual const std::string& GetID(void) const { return (id_); }

    // Return the reference frame's pose relative to its parent
    virtual Pose GetLocalPose(void) const { return Pose(); }

    // Return the transform tree to which this reference frame is registered
    virtual TransformTree* GetTransformTree(void) const {
      return (ptransformtree_parent_);
    }

    // Set the reference frame's ID
    virtual void SetID(const std::string& id) { id_ = id; }

    // Move assignment
    RefFrame& operator=(RefFrame&& ref_frame_other) noexcept;

    // Protected Friends
   protected:
    friend class TransformTree;

    // Protected Types
   protected:
    class PoseCache {
     public:
      // Returns whether the cached global-frame pose is valid
      bool FIsValid(void) const noexcept { return (fposes_are_valid_); }

      // Return the cached global-frame pose
      const Pose& Get(void) const noexcept { return (pose_); }

      // Return the cached inverse global-frame pose
      const Pose& GetInverse(void) const noexcept { return (pose_inverse_); }

      // Invalidate the global-frame pose cache
      void Invalidate(void) noexcept { fposes_are_valid_ = false; }

      // Set the global-frame pose cache
      void Set(const Pose& pose);

     protected:
      bool fposes_are_valid_ = false;  // If true, poses are valid
      Pose pose_;          // If fposes_are_valid_, cache of RefFrame's pose
      Pose pose_inverse_;  // If fposes_are_valid_, cache of inverse of
                           // RefFrame's pose
    };                     // class GlobalPoseCache

    // Protected Methods
   protected:
    // Return the cache of this frame's pose in the global frame
    PoseCache& GetGlobalPoseCache(void) const { return (*ppose_cache_global_); }

    // Protected Data
   protected:
    std::string id_;  // ID of this RefFrame
    std::unique_ptr<PoseCache>
        ppose_cache_global_;  // Cache of this RefFrame's global pose
    TransformTree*
        ptransformtree_parent_;  // When non-null, transformtree to which this
                                 // reference frame is registered
  };                             // class RefFrame

  //--------------------------------------------------------------------------
  // Class ConstRefFrame
  //
  // This subclass of RefFrame also stores the reference frame's local pose
  // which cannot be changed.  This is useful for reference frames whose pose
  // never changes and has better performance than StaticRefFrame.
  //
  // This class is multithread-safe.
  //--------------------------------------------------------------------------
  class ConstRefFrame : public RefFrame {
    // Public Methods
   public:
    ConstRefFrame(const std::string& id) noexcept : RefFrame(id), pose_() {}
    ConstRefFrame(const std::string& id, const Pose& pose) noexcept
        : RefFrame(id), pose_(pose) {}
    ConstRefFrame(const std::string& id, Pose&& pose) noexcept
        : RefFrame(id), pose_(std::move(pose)) {}
    ConstRefFrame(ConstRefFrame&& const_ref_frame_other) noexcept
        : RefFrame(std::move(const_ref_frame_other)),
          pose_(std::move(const_ref_frame_other.pose_)) {}

    // Move assignment
    ConstRefFrame& operator=(ConstRefFrame&& const_ref_frame_other) noexcept;

    // Public RefFrame Method Overrides
   public:
    virtual Pose GetLocalPose(void) const override { return (pose_); }

    // Protected Data
   protected:
    Pose pose_;  // Reference frame's pose
  };             // class ConstRefFrame

  //--------------------------------------------------------------------------
  // Class IndirectRefFrame
  //
  // This subclass of RefFrame obtains the reference frame's local pose
  // through a pointer to the pose.  This allows the pose to be updated
  // externally without having to also explicitly update the associated
  // RefFrame.
  //
  // Caution:  This class does not ensure multithread-safe access to the
  // pose.  Users may wish to create a subclass that uses an appropriate
  // locking mechanism.
  //--------------------------------------------------------------------------
  class IndirectRefFrame : public RefFrame {
    // Public Methods
   public:
    IndirectRefFrame(const std::string& id, Pose* ppose) noexcept
        : RefFrame(id), ppose_(ppose) {}
    IndirectRefFrame(IndirectRefFrame&& indirect_ref_frame_other) noexcept
        : RefFrame(std::move(indirect_ref_frame_other)),
          ppose_(std::move(indirect_ref_frame_other.ppose_)) {}

    // Set the pointer to the RefFrame's pose
    void SetPosePtr(const Pose* ppose) { ppose_ = ppose; }

    // Move assignment
    IndirectRefFrame& operator=(
        IndirectRefFrame&& indirect_ref_frame_other) noexcept;

    // Public RefFrame Method Overrides
   public:
    virtual Pose GetLocalPose(void) const override { return (*ppose_); }

    // Protected Data
   protected:
    const Pose* ppose_;  // Pointer to reference frame's pose
  };                     // class IndirectRefFrame

  //--------------------------------------------------------------------------
  // Class TransformRefFrame
  //
  // This subclass of RefFrame obtains the reference frame's local pose
  // through a pointer to a Transform object.  This allows the pose to be
  // updated externally without having to also explicitly update the
  // associated RefFrame.
  //
  // Caution:  This class does not ensure multithread-safe access to the
  // transform object.  Users may wish to create a subclass that uses an
  // appropriate locking mechanism.
  //--------------------------------------------------------------------------
  class TransformRefFrame : public RefFrame {
    // Public Methods
   public:
    TransformRefFrame(const std::string& id, Transform* ptransform) noexcept
        : RefFrame(id), ptransform_(ptransform) {}
    TransformRefFrame(TransformRefFrame&& transform_ref_frame_other) noexcept
        : RefFrame(std::move(transform_ref_frame_other)),
          ptransform_(std::move(transform_ref_frame_other.ptransform_)) {}

    // Set the pointer to the RefFrame's pose
    void SetTransformPtr(const Transform* ptransform) {
      ptransform_ = ptransform;
    }

    // Move assignment
    TransformRefFrame& operator=(
        TransformRefFrame&& transform_ref_frame_other) noexcept;

    // Public RefFrame Method Overrides
   public:
    virtual Pose GetLocalPose(void) const override {
      return (Pose(ptransform_->translation_, ptransform_->rotation_));
    }

    // Protected Data
   protected:
    const Transform* ptransform_;  // Pointer to reference frame's transform
  };                               // class TransformRefFrame

  //--------------------------------------------------------------------------
  // Class StaticRefFrame
  //
  // This subclass of RefFrame also stores the reference frame's local pose.
  // This is useful for reference frames whose pose changes infrequently.
  //
  // This class is multithread-safe.
  //--------------------------------------------------------------------------
  class StaticRefFrame : public RefFrame {
    // Public Methods
   public:
    StaticRefFrame(const std::string& id) noexcept
        : RefFrame(id), mutex_(), pose_() {}
    StaticRefFrame(const std::string& id, const Pose& pose) noexcept
        : RefFrame(id), mutex_(), pose_(pose) {}
    StaticRefFrame(const std::string& id, Pose&& pose) noexcept
        : RefFrame(id), mutex_(), pose_(std::move(pose)) {}
    StaticRefFrame(StaticRefFrame&& static_ref_frame_other) noexcept
        : RefFrame(std::move(static_ref_frame_other)),
          mutex_(),
          pose_(std::move(static_ref_frame_other.pose_)) {}

    // Set the local pose
    void SetLocalPose(const Pose& pose) {
      std::lock_guard<std::mutex> lg(mutex_);
      pose_ = pose;
    }

    // Move assignment
    StaticRefFrame& operator=(StaticRefFrame&& static_ref_frame_other) noexcept;

    // Public RefFrame Method Overrides
   public:
    virtual Pose GetLocalPose(void) const override {
      std::lock_guard<std::mutex> lg(mutex_);
      return (pose_);
    }

    // Protected Dataq
   protected:
    mutable std::mutex mutex_;  // Access guard to m_pose
    Pose pose_;                 // Reference frame's pose
  };                            // class StaticRefFrame

  // Public Constants
 public:
  static const ConstRefFrame
      kRefFrameGlobal;  // Predefined reference frame object representing global
                        // coordinates

  // Public Methods
 public:
  TransformTree(void) noexcept;
  ~TransformTree();

  //--------------------------------------------------------------------------
  // Convert
  //
  // Converts a pose from one reference frame to another.
  //
  // Arguments:
  //    const Pose &poseFrom           The pose to convert
  //    const RefFrame &ref_frameFrom  The reference frame of poseFrom
  //    const RefFrame &ref_frameTo    The reference frame in which we want the
  //                                       equivalent of poseFrom
  //    Pose *pposeTo                  The buffer to get the equivalent of
  //                                       poseFrom in ref_frameTo; can be same
  //                                       object as poseFrom
  //
  // Return:
  //    (Return)       true on success, false otherwise (such as ref_frameFrom
  //                       and ref_frameTo don't have a common ancestor parent)
  //    Pose *pposeTo  On success, the equivalent of poseFrom in ref_frameTo
  //--------------------------------------------------------------------------
  bool Convert(const Pose& poseFrom, const RefFrame& ref_frameFrom,
               const RefFrame& ref_frameTo, Pose* pposeTo) const;

  //--------------------------------------------------------------------------
  // Register
  //
  // Register a reference frame.  Reference frames must be registered before
  // they can be used by Convert().  Reference frames can be registered to only
  // one TransformTree at a time.
  //
  // Arguments:
  //    RefFrame *pref_frame_inout       Reference frame to register
  //    const RefFrame &ref_frameParent  Parent reference frame of
  //                                         *pref_frame_inout
  //--------------------------------------------------------------------------
  void Register(RefFrame* pref_frame_inout, const RefFrame& ref_frameParent);

  //--------------------------------------------------------------------------
  // Register
  //
  // Register a reference frame.  Reference frames must be registered before
  // they can be used by Convert().  Reference frames can be registered to only
  // one TransformTree at a time.
  //
  // This is a convenience function and otherwise identical to calling
  // Register() with a pointer to the RefFrame.
  //
  // Arguments:
  //    RefFrame &ref_frame_inout        Reference frame to register
  //    const RefFrame &ref_frameParent  Parent reference frame of
  //    &ref_frame_inout
  //--------------------------------------------------------------------------
  void Register(RefFrame& ref_frame_inout, const RefFrame& ref_frameParent);

  //--------------------------------------------------------------------------
  // StartCachedMode
  //
  // Start or restart caching and using cached poses for faster pose
  // conversions.  The existing cache is invalidated first.
  //
  // The first time a RefFrame is queried during a pose conversion, the
  // RefFrame's pose is cached and won't be queried until after StopCachedMode
  // is or StartCached is called again.
  //
  // If the TransformTree is always used in cached mode, StopCachedMode()
  // need not be called.  just call StartCacheMode again whenever the cache
  // should be invalidated.
  //--------------------------------------------------------------------------
  void StartCachedMode(void);

  //--------------------------------------------------------------------------
  // StopCachedMode
  //
  // Stop caching and using cached poses.  Pose conversions always query
  // RefFrames for their current local pose.
  //--------------------------------------------------------------------------
  void StopCachedMode(void) { fdo_cached = false; }

  //--------------------------------------------------------------------------
  // Unregister
  //
  // Unregister a reference frame.  The reference frame can then be registered
  // again or to another TransformTree.
  //
  // The RefFrame is also removed from the parent chain of any descendant
  // RefFrames.  Those RefFrame's are "orphaned" in that they lose the link to
  // the root RefFrame.  They should be unregistered and reregistered with
  // new parents.
  //
  // Arguments:
  //    RefFrame pref_frame  Reference frame to unregister
  //--------------------------------------------------------------------------
  void Unregister(RefFrame* pref_frame);

  //--------------------------------------------------------------------------
  // Unregister
  //
  // Unregister a reference frame.  The reference frame can then be registered
  // to another TransformTree.
  //
  // The RefFrame are also removed from the parent chain of any descendant
  // RefFrames.  Those RefFrame's are "orphaned" in that they lose the link to
  // the root RefFrame.  They should be unregistered and reregistered with
  // new parents.
  //
  // This is a convenience function and otherwise identical to calling
  // Unregister() with a pointer to the RefFrame.
  //
  // Arguments:
  //    RefFrame &ref_frame  Reference frame to unregister
  //--------------------------------------------------------------------------
  void Unregister(RefFrame& ref_frame);

  // Public Debug Methods
 public:
  // Print the transform tree to the debug output
  void Dump(void);

  // Protected Types
 protected:
  typedef std::vector<RefFrameID>
      VecRefFrameID;  // Array of reference frame ID's
  typedef std::unordered_map<RefFrameID, VecRefFrameID>
      MapRefFrameIDVecRefFrameID;  // Mapping from RefFrame ID to parent chain
                                   // in leaf-to-root order

  // Protected Constants
 protected:
  static const RefFrameID
      kRefFrameIDGlobal;  // ID of the global reference frame

  // Protected Class-Wide Methods
 protected:
  // Ensure that the cache of the RefFrame and it's ancestors are valid
  static void EnsureParentChainCache(RefFrameID ref_frame_id_to,
                                     const RefFrameID* pref_frame_id_to_first,
                                     const RefFrameID* pref_frame_id_to_last);

  // Combine two poses.  With non-inverted poses this maps the first pose in
  // the second pose's frame to a pose in the second's parent frame
  static void CombinePose(Pose* ppose_inout, const Pose& pose2);

  // Combine a pose with the inversion of the second.  With non-inserted poses
  // this maps the first pose in the second's parent frame to the second's frame
  static void CombinePoseInverse(Pose* ppose_inout, const Pose& pose2);

  // Maps a pose from a RefFrame to its immediate parent RefFrame
  static void MapPoseFromRefFrame(RefFrameID ref_frame_id_from,
                                  Pose* ppose_inout);

  // Maps a pose from the RefFrame's parent to the RefFrame
  static void MapPoseToRefFrame(RefFrameID ref_frame_id_to, Pose* ppose_inout);

  // Convert a RefFrame ID to the RefFrame
  static RefFrame* ToRefFrame(RefFrameID ref_frame_id) {
    return (reinterpret_cast<RefFrame*>(ref_frame_id));
  }

  // Convert a RefFrame to a RefFrame ID
  static RefFrameID ToRefFrameID(RefFrame* pref_frame) {
    return (reinterpret_cast<RefFrameID>(pref_frame));
  }
  static RefFrameID ToRefFrameID(const RefFrame* pref_frame) {
    return (reinterpret_cast<RefFrameID>(pref_frame));
  }

  // Protected Methods
 protected:
  // Map the pose by following the RefFrame chains
  static void MapPose(Pose* ppose_inout, RefFrameID pref_frame_id_from,
                      const RefFrameID* pref_frame_id_from_first,
                      const RefFrameID* pref_frame_id_from_last,
                      RefFrameID ref_frame_id_to,
                      const RefFrameID* pref_frame_id_to_first,
                      const RefFrameID* pref_frame_id_to_last);

  // Converts a registration from one RefFrame to another.  The old RefFrame
  // is no longer registered in this TransformTree and the new RefFrame is
  // registered in its place, including taking over the parent chain and
  // taking the place of the old RefFrame in the parent chains of other
  // RefFrames.
  void TransferRegistration(RefFrame* pref_frame_old, RefFrame* pref_frame_new);

  // Protected Data
 protected:
  bool fdo_cached;  // If true, use cached and cache on first reference RefFrame
                    // global poses
  MapRefFrameIDVecRefFrameID
      map_ref_frame_id_vecref_frame_id_;  // Mapping from ref_frame ID to
                                          // ref_frame ID parent chain
  std::mutex mutex_map_;  // Access guard to map_ref_frame_id_vecref_frame_id_
};                        // class TransformTree

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_TRANSFORMS_TRANSFORM_TREE_HPP_
