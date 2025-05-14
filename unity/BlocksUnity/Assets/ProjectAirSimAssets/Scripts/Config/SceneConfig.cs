// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

using Newtonsoft.Json;
using Newtonsoft.Json.Schema;
using System;
using System.ComponentModel;
using System.IO;
using System.Text;

namespace UnityProjectAirSim.Config
{
    // TODO: add JsonRequired attribute to all required fields.
    [JsonObject(MemberSerialization.OptOut)]
    public class SceneConfig
    {
        [JsonProperty(PropertyName = "id")]
        public string Id
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "actors")]
        public ActorConfig[] Actors
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "clock")]
        public ClockConfig Clock
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "home-geo-point")]
        public HomeGeoPointConfig HomeGeoPoint
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "segmentation")]
        public SegmentationConfig Segmentation
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "scene-type")]  // TODO allow defaults
        public string SceneType
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "tiles-dir")]
        public string TilesDir
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "tiles-dir-is-client-relative")]
        public bool TilesDirIsClientRelative
        {
            get;
            set;
        }
    }

    // TODO: move to their own files?
    [JsonObject(MemberSerialization.OptOut)]
    public class ActorConfig
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

        [JsonProperty(PropertyName = "origin")]
        public PoseConfig Origin
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "robot-config")]
        public RobotConfig RobotConfig
        {
            get;
            set;
        }
    }

    // Custom converter to split config string into 3 floats
    public class VectorConverter : JsonConverter<InteropVector3>
    {
        public override void WriteJson(JsonWriter writer, InteropVector3 vec,
                                       JsonSerializer serializer)
        {
            writer.WriteValue($"{vec.x} {vec.y} {vec.z}");
        }

        public override InteropVector3 ReadJson(JsonReader reader, Type objectType,
                                                InteropVector3 existingValue,
                                                bool hasExistingValue,
                                                JsonSerializer serializer)
        {
            var value = (string)reader.Value;
            if (value.Length < 5)  // at least 3 digits + 2 spaces
            {
                return new InteropVector3(0, 0, 0);
            }

            string[] strNums = value.Trim().Split(' ');
            var vec = new InteropVector3
            {
                x = float.Parse(strNums[0]),
                y = float.Parse(strNums[1]),
                z = float.Parse(strNums[2]),
            };

            return vec;
        }
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class PoseConfig
    {
        [JsonProperty(PropertyName = "xyz")]
        [JsonConverter(typeof(VectorConverter))]
        public InteropVector3 Position
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "rpy")]
        [JsonConverter(typeof(VectorConverter))]
        public InteropVector3 Rotation
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "rpy-deg")]
        [JsonConverter(typeof(VectorConverter))]
        public InteropVector3 RotationDeg
        {
            get;
            set;
        }
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class ClockConfig
    {
        [JsonProperty(PropertyName = "steppable")]
        public bool Steppable
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "step-ns")]
        public int StepNS
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "real-time-update-rate")]
        public int RealTimeUpdateRate
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "pause-on-start")]
        public bool PauseOnStart
        {
            get;
            set;
        }
    }

    [JsonObject(MemberSerialization.OptOut)]
    public class HomeGeoPointConfig
    {
        [JsonProperty(PropertyName = "latitude")]
        public double Latitude
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "longitude")]
        public double Longitude
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "altitude")]
        public float Altitude
        {
            get;
            set;
        }
    }

    // TODO: move to their own files
    [JsonObject(MemberSerialization.OptOut)]
    public class SegmentationConfig
    {
        [JsonProperty(PropertyName = "initialize-ids")]
        public bool InitializeIds
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "ignore-existing")]
        public bool IgnoreExisting
        {
            get;
            set;
        }

        [JsonProperty(PropertyName = "use-owner-name")]
        public bool UseOwnerName
        {
            get;
            set;
        }
    }
}