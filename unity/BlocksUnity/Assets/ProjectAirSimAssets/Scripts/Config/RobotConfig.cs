// Copyright (C) Microsoft Corporation. All rights reserved.

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.ComponentModel;

namespace UnityProjectAirSim.Config
{
    [JsonObject(MemberSerialization.OptOut)]
    public class RobotConfig
    {
        [JsonProperty(PropertyName = "physics-type",
                      ItemTypeNameHandling = TypeNameHandling.All)]
        public string PhysicsType
        {
            get;
            set;
        }

        //[JsonArray()]
        [JsonProperty(PropertyName = "links")]
        public LinkConfig[] Links
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "joints")]
        public JointConfig[] Joints
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "sensors",
                      ItemTypeNameHandling = TypeNameHandling.All)]
        public SensorConfig[] Sensors
        {
            get;
            set;
        }
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class LinkConfig
    {
        [JsonProperty(PropertyName = "name")]
        public string Name
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "collision")]
        public CollisionConfig Collision
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "visual")]
        public VisualConfig Visual
        {
            get;
            set;
        }

        // TODO: add intertial
        // TODO: add aerodynamics
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class CollisionConfig
    {
        [JsonProperty(PropertyName = "enabled",
                      DefaultValueHandling = DefaultValueHandling.Populate)]
        [DefaultValue(true)]
        public bool IsEnabled
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "restitution",
                      DefaultValueHandling = DefaultValueHandling.Populate)]
        [DefaultValue(0F)]
        public float Restitution
        {
            get;
            set;
        }

        // TODO: I think I'm missing restitution and friction properties
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class VisualConfig
    {
        [JsonProperty(PropertyName = "geometry")]
        public GeometryConfig Geometry
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "origin")]  // TODO: add default
        public PoseConfig Origin
        {
            get;
            set;
        }
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class GeometryConfig
    {
        [JsonProperty(PropertyName = "type")]
        public string Type
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "name")]
        public string Name
        {
            get;
            set;
        }
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class JointConfig
    {
        [JsonProperty(PropertyName = "id")]
        public string Id
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "origin")]
        public PoseConfig Origin
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "type")]
        public string Type
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "parent-link")]
        public string ParentLink
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "child-link")]
        public string ChildLink
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "axis")]
        [JsonConverter(typeof(VectorConverter))]
        public InteropVector3 Axis
        {
            get;
            set;
        }
    }

    // A temporary hack to be able to load Sensor vs Camera config, since
    // TypeNameHandling wasnt working to deserialize as derived types.
    public class SensorJsonConverter : JsonConverter<SensorConfig>
    {
        public override void WriteJson(JsonWriter writer, SensorConfig sensorConfig,
                                       JsonSerializer serializer)
        {
            throw new System.NotImplementedException();
        }

        public override SensorConfig ReadJson(JsonReader reader,
                                              System.Type objectType,
                                              SensorConfig existingValue,
                                              bool hasExistingValue,
                                              JsonSerializer serializer)
        {
            var j = JObject.Load(reader);

            var sensorConfig = new SensorConfig
            {
                Id = j["id"]
                                                         .ToString(),
                Type = j["type"]
                                                           .ToString()
            };
            sensorConfig.JsonString = j.ToString();

            return sensorConfig;
        }
    }

    [JsonObject(MemberSerialization.OptOut,
                ItemTypeNameHandling = TypeNameHandling.All)]
    [JsonConverter(typeof(SensorJsonConverter))]
    public class SensorConfig
    {
        public string JsonString;

        [JsonProperty(PropertyName = "id")]
        public string Id
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "type")]
        public string Type
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "enabled")]
        public bool IsEnabled
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "parent-link")]
        public string ParentLink
        {
            get;
            set;
        }
    }

    [JsonObject(MemberSerialization.OptOut,
                ItemTypeNameHandling = TypeNameHandling.All)]
    public class CameraConfig
    {
        // Sensor config
        [JsonProperty(PropertyName = "id")]
        public string Id
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "type")]
        public string Type
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "enabled")]
        public bool IsEnabled
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "parent-link")]
        public string ParentLink
        {
            get;
            set;
        }

        // Camera config
        [JsonProperty(PropertyName = "capture-interval")]
        public float CaptureInterval
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "focal-length")]
        public float FocalLength
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "capture-settings")]
        public CameraCaptureConfig[] CaptureSettings
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "gimbal")]
        public GimbalConfig Gimbal
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "origin")]
        public PoseConfig Origin
        {
            get;
            set;
        }
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class CameraCaptureConfig
    {
        [JsonProperty(PropertyName = "image-type")]
        public ImageType ImageType
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "width")]
        public int Width
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "height")]
        public int Height
        {
            get;
            set;
        }

        [JsonProperty("fov-degrees")]
        public int FovDegrees
        {
            get;
            set;
        }

        [JsonProperty("capture-enabled")]
        public bool CaptureEnabled
        {
            get;
            set;
        }

        [JsonProperty("streaming-enabled")]
        public bool StreamingEnabled
        {
            get;
            set;
        }

        [JsonProperty("pixels-as-float")]
        public bool PixelsAsFloat
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "Compress")]
        public bool Compress
        {
            get;
            set;
        }

        [JsonProperty("target-gamma")]
        public double TargetGamma
        {
            get;
            set;
        }
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class GimbalConfig
    {
        [JsonProperty(PropertyName = "lock-roll")]
        public bool LockRoll
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "lock-pitch")]
        public bool LockPitch
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "lock-yaw")]
        public bool LockYaw
        {
            get;
            set;
        }
    }

    [JsonObject(MemberSerialization.OptOut,
                ItemTypeNameHandling = TypeNameHandling.All)]
    public class LidarConfig
    {
        // Sensor config
        [JsonProperty(PropertyName = "id")]
        public string Id
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "type")]
        public string Type
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "enabled")]
        public bool IsEnabled
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "parent-link")]
        public string ParentLink
        {
            get;
            set;
        }

        // Lidar config
        [JsonProperty(PropertyName = "origin")]
        public PoseConfig Origin
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "number-of-channels")]
        public int NumberOfChannels
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "range")]
        public float Range
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "points-per-second")]
        public int PointsPerSecond
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "report-frequency")]
        public float ReportFrequency
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "horizontal-rotation-frequency")]
        public int HorizontalRotationFrequency
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "horizontal-fov-start-deg")]
        public float HorizontalFovStartDeg
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "horizontal-fov-end-deg")]
        public float HorizontalFovEndDeg
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "vertical-fov-upper-deg")]
        public float VerticalFovUpperDeg
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "vertical-fov-lower-deg")]
        public float VerticalFovLowerDeg
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "draw-debug-points")]
        public bool DrawDebugPoints
        {
            get;
            set;
        }
    }
}