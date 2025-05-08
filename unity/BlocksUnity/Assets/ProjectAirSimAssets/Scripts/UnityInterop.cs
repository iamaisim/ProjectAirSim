// Copyright (C) Microsoft Corporation. All rights reserved.

using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace UnityProjectAirSim
{

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropVector3
    {
        public float x;
        public float y;
        public float z;

        public InteropVector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public void Set(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public void Reset() { x = y = z = 0; }

        public static InteropVector3 Zero
        {
            get { return new InteropVector3(0.0f, 0.0f, 0.0f); }
        }

        public static bool operator ==(InteropVector3 v1, InteropVector3 v2)
        {
            return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
        }

        public static bool operator !=(InteropVector3 v1, InteropVector3 v2)
        {
            return !(v1 == v2);
        }

        public override bool Equals(object obj)
        {
            // Check for null and type
            if ((obj == null) || !this.GetType().Equals(obj.GetType()))
            {
                return false;
            }
            else // Check for matching data values
            {
                InteropVector3 v = (InteropVector3)obj;
                return (x == v.x) && (y == v.y) && (z == v.z);
            }
        }

        public override int GetHashCode()
        {
            // Generic integer hash for x,y,z float values
            int hashcode = 23;
            hashcode = (hashcode * 37) + x.GetHashCode();
            hashcode = (hashcode * 37) + y.GetHashCode();
            hashcode = (hashcode * 37) + z.GetHashCode();
            return hashcode;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropQuaternion
    {
        public float x;
        public float y;
        public float z;
        public float w;

        public InteropQuaternion(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public void Set(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public void Reset() { x = y = z = w = 0; }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropPose
    {
        public InteropVector3 position;
        public InteropQuaternion orientation;

        public InteropPose(InteropVector3 position, InteropQuaternion orientation)
        {
            this.position = position;
            this.orientation = orientation;
        }

        public void Reset()
        {
            position.Reset();
            orientation.Reset();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropTwist
    {
        public InteropVector3 linear;
        public InteropVector3 angular;

        public InteropTwist(InteropVector3 linear, InteropVector3 angular)
        {
            this.linear = linear;
            this.angular = angular;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropAccelerations
    {
        public InteropVector3 linear;
        public InteropVector3 angular;

        public InteropAccelerations(InteropVector3 linear, InteropVector3 angular)
        {
            this.linear = linear;
            this.angular = angular;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropKinematics
    {
        public InteropPose pose;
        public InteropTwist twist;
        public InteropAccelerations accels;

        public InteropKinematics(InteropPose pose, InteropTwist twist,
                                 InteropAccelerations accels)
        {
            this.pose = pose;
            this.twist = twist;
            this.accels = accels;
        }
    }

    public enum ApplyOrder
    {
        Pre = 0,
        PreTranslation = 1,
        Post = 2
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropActuatedTransform
    {
        public InteropPose actuated_transform;
        public ApplyOrder apply_order;
        public IntPtr actuated_link_id;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropCollisionInfo
    {
        [MarshalAs(UnmanagedType.U1)]
        public bool has_collided;

        public InteropVector3 normal;
        public InteropVector3 impact_point;
        public InteropVector3 position;
        public float penetration_depth;
        public Int64 time_stamp;
        public string object_name;
        public int segmentation_id;

        public void SetDefaultValues()
        {
            has_collided = false;
            normal = new InteropVector3(0, 0, 0);
            impact_point = new InteropVector3(0, 0, 0);
            position = new InteropVector3(0, 0, 0);
            penetration_depth = 0;
            time_stamp = 0;
            object_name = "none";
            segmentation_id = -1;
        }
    }

    public enum ImageType
    {
        Scene = 0,
        DepthPlanar = 1,
        DepthPerspective = 2,
        Segmentation = 3,
        DepthVis = 4,
        DisparityNormalized = 5,
        SurfaceNormals = 6,
        // Infrared = 7, // not implemented
        Count  //  must be last
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct InteropImageMessage
    {
        public Int64 time_stamp;
        public uint height;
        public uint width;
        public string encoding;

        [MarshalAs(UnmanagedType.U1)]
        public bool big_endian;

        public uint step;
        public byte[] image_data_uint;
        public InteropVector3 pos;
        public InteropQuaternion rot;

        public InteropImageMessage(Int64 time_stamp, uint height, uint width,
                                   string encoding, bool big_endian, uint step,
                                   byte[] image_data_uint, InteropVector3 pos,
                                   InteropQuaternion rot)
        {
            this.time_stamp = time_stamp;
            this.height = height;
            this.width = width;
            this.encoding = encoding;
            this.big_endian = big_endian;
            this.step = step;
            this.image_data_uint = image_data_uint;
            this.pos = pos;
            this.rot = rot;
        }

        public void Reset()
        {
            time_stamp = 0;
            height = 0;
            width = 0;
            encoding = "BGR";
            big_endian = false;
            step = 0;
            image_data_uint = new byte[] { };
            pos = new InteropVector3(0, 0, 0);
            rot = new InteropQuaternion(0, 0, 0, 0);
        }
    }


    [StructLayout(LayoutKind.Sequential)]
    public struct InteropLidarMessage
    {
        public Int64 time_stamp;
        public int num_points;
        public float[] point_cloud;
        public int[] segmentation_cloud;
        public float[] intensity_cloud;

        public int[] laser_index_cloud;
        public InteropPose pose;

        public InteropLidarMessage(Int64 time_stamp, int num_points,
                                   float[] point_cloud,
                                   int[] segmentation_cloud,
                                   float[] intensity_cloud, int[] laser_index_cloud, InteropPose pose)
        {
            this.time_stamp = time_stamp;
            this.num_points = num_points;
            this.point_cloud = point_cloud;
            this.segmentation_cloud = segmentation_cloud;
            this.intensity_cloud = intensity_cloud;
            this.laser_index_cloud = laser_index_cloud;
            this.pose = pose;
        }

        public void Reset()
        {
            time_stamp = 0;
            num_points = 0;
            point_cloud = new float[] { };
            segmentation_cloud = new int[] { };
            intensity_cloud = new float[] { };
            laser_index_cloud = new int[] { };
            pose = new InteropPose();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct TileKey
    {
        public int x;
        public int y;

        public int lod;

        public TileKey(int x, int y, int lod)
        {
            this.x = x;
            this.y = y;
            this.lod = lod;
        }
    }
}
