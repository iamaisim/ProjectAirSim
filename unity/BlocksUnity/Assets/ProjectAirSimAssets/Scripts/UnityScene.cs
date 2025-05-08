// Copyright (C) Microsoft Corporation. All rights reserved.

using Newtonsoft.Json;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using UnityProjectAirSim.Config;
using UnityProjectAirSim.Rendering.Scene;
using UnityProjectAirSim.Robot;
using UnityProjectAirSim.Sensors;
using UnityProjectAirSim.World;
using UnityEngine;
using UnityGLTF;
using UnityGLTF.Loader;

namespace UnityProjectAirSim
{
    public class UnityScene : MonoBehaviour
    {
        private static List<GameObject> Robots = new List<GameObject>();
        GISRenderer GISRendererComponent;

        void Awake() { }

        // Start is called before the first frame update
        void Start() { }

        // Update is called once per frame
        void Update() { }

        public void LoadUnityScene()
        {
            string sceneConfigJSON =
                Marshal.PtrToStringAnsi(PInvokeWrapper.GetSceneConfigJSON());
            var sceneConfig =
                JsonConvert.DeserializeObject<SceneConfig>(sceneConfigJSON);

            if (sceneConfig.Actors != null)
            {
                foreach (var actorConfig in sceneConfig.Actors)
                {
                    if (actorConfig.Type != "robot")
                    {
                        continue;
                    }

                    var robotObject = new GameObject(actorConfig.Name, typeof(UnityRobot));
                    var robotComponent = robotObject.GetComponent<UnityRobot>();
                    robotComponent.Initialize(actorConfig.RobotConfig, actorConfig.Name,
                                              actorConfig.Origin);

                    Robots.Add(robotObject);
                    WorldSimApi.AttachWeatherFXToRobot(robotObject);
                }
            }

            // Initialize segmentation object IDs
            InitSegmentationIDs(sceneConfig);

            if (!string.IsNullOrWhiteSpace(sceneConfig.TilesDir))
            {
                GISRendererComponent = gameObject.AddComponent<GISRenderer>();
                GISRendererComponent.Initialize(sceneConfig.TilesDir, Robots);
            }
        }

        public void UnloadUnityScene()
        {
            foreach (var robot in Robots)
            {
                Destroy(robot);
            }
            Robots.Clear();

            if (GISRendererComponent != null)
            {
                Destroy(GISRendererComponent);
            }
        }

        void OnDestroy() { }

        private void LoadUnityEnvActor()
        {
            // TODO
        }

        private void InitSegmentationIDs(SceneConfig sceneConfig)
        {
            CameraFiltersScript.InitializeSegmentationIdsDict();

            if (sceneConfig.Segmentation == null ||
                !sceneConfig.Segmentation.InitializeIds) return;

            // Re-initialize segmentation IDs as hash of their names
            var renderers = FindObjectsOfType<Renderer>();
            foreach (var r in renderers)
            {
                var objectId = r.gameObject.GetInstanceID();
                var objectName = r.gameObject.name;
                // TODO Support use-owner-name to find root object name
                // TODO Support ignore-existing to only set new IDs for un-set ones

                var segId = CameraFiltersScript.CalculateSegmentationId(objectName);

                bool result = CameraFiltersScript.SetSegmentationId(objectName, segId, false);
            }
        }
    }
}