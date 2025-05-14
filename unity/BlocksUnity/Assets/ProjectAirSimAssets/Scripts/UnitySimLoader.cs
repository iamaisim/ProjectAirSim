// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

using System.Runtime.InteropServices;
using System.Threading;

using UnityProjectAirSim.World;

using UnityEngine;

namespace UnityProjectAirSim
{
    public class UnitySimLoader : MonoBehaviour
    {
        UnityScene UnitySceneComponent;

        delegate void LoadExternalSceneDelegate();
        LoadExternalSceneDelegate LoadExternalSceneDelegateInstance;

        delegate void StartExternalSceneDelegate();
        StartExternalSceneDelegate StartExternalSceneDelegateInstance;

        delegate void StopExternalSceneDelegate();
        StopExternalSceneDelegate StopExternalSceneDelegateInstance;

        delegate void UnloadExternalSceneDelegate();
        UnloadExternalSceneDelegate UnloadExternalSceneDelegateInstance;

        void Awake()
        {
            int topicsPort = LoadTopicsPort();
            int servicesPort = LoadServicesPort();

            // Load simulator and coresim scene
            PInvokeWrapper.LoadServer(topicsPort, servicesPort);

            // TODO: I dont think we need these game objects, we can just create a
            // UnityScene class right?
            var sceneObject = new GameObject(
                "ProjectAirSimScene",
                typeof(UnityScene));  // TODO: replace name with name in config?
                                      // sceneObject.AddComponent<GISRenderer>(); // TODO
            UnitySceneComponent = sceneObject.GetComponent<UnityScene>();

            // Load initial scene
            UnitySceneComponent.LoadUnityScene();

            // Hook up callbacks for scene reloading
            LoadExternalSceneDelegateInstance = this.LoadUnityScene;
            PInvokeWrapper.SetCallbackLoadExternalScene(
                Marshal.GetFunctionPointerForDelegate(
                    LoadExternalSceneDelegateInstance));

            StartExternalSceneDelegateInstance = this.StartUnityScene;
            PInvokeWrapper.SetCallbackStartExternalScene(
                Marshal.GetFunctionPointerForDelegate(
                    StartExternalSceneDelegateInstance));

            StopExternalSceneDelegateInstance = this.StopUnityScene;
            PInvokeWrapper.SetCallbackStopExternalScene(
                Marshal.GetFunctionPointerForDelegate(
                    StopExternalSceneDelegateInstance));

            UnloadExternalSceneDelegateInstance = this.UnloadUnityScene;
            PInvokeWrapper.SetCallbackUnloadExternalScene(
                Marshal.GetFunctionPointerForDelegate(
                    UnloadExternalSceneDelegateInstance));

            // Start simulator and scene
            PInvokeWrapper.StartServer();

            StartUnityScene();
        }

        void Update() { }

        int LoadTopicsPort()
        {
            var topicsPort = 8989;
            // TODO: read command line arg topicsport

            return topicsPort;
        }

        int LoadServicesPort()
        {
            var servicesPort = 8990;
            // TODO: read command line arg servicesport

            return servicesPort;
        }

        public void LoadUnityScene()
        {
            GameThreadDispatcher.RunCommandOnGameThread(
                () =>
                {
                    // TODO Respawn UnitySceneComponent?

                    // First initialize WorldSimApi to enable attaching
                    // WeatherFX components to robots when loading the scene.
                    WorldSimApi.Initialize();
                    UnitySceneComponent.LoadUnityScene();
                    // Hook up world APIs after UnityScene is done loading.
                    WorldSimApi.RegisterServiceMethods();
                },
                waitForCompletion: true);
        }

        public void StartUnityScene()
        {
            // TODO: handle pause before starting?
        }

        public void StopUnityScene()
        {
            // TODO
        }

        public void UnloadUnityScene()
        {
            GameThreadDispatcher.RunCommandOnGameThread(
                () =>
                {
                    WorldSimApi.Unload();
                    UnitySceneComponent.UnloadUnityScene();

                    // TODO Destroy UnitySceneComponent?
                },
                waitForCompletion: true);
        }

        void OnDestroy() { PInvokeWrapper.StopServer(); }
    }
}