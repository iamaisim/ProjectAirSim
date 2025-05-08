
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using UnityEngine;

using UnityProjectAirSim.Weather;


namespace UnityProjectAirSim.World
{
    /// <summary>
    /// WorldSimApi class that hooks callbacks upon initialization.
    ///
    /// To add a new method:
    /// 1. Add method signature for hooking callback in PInvokeWrapper.cs, e.g. SetSpawnObjectCallback
    /// 2. Implement method in this class with interop types, e.g. SpawnObjectOnGameThread
    /// 3. Define and instantiate delegate, e.g. SpawnObjectDelegateInstance
    /// 4. Hook callback in Start() through PInvokeWrapper method
    /// </summary>
    public class WorldSimApi : MonoBehaviour
    {
        delegate string SpawnObjectDelegate(string name, string assetPath,
            InteropPose pose, InteropVector3 scale);
        SpawnObjectDelegate SpawnObjectDelegateInstance = SpawnObjectOnGameThread;

        delegate string SpawnObjectAtGeoDelegate(string name, string assetPath,
            InteropPose pose, InteropVector3 scale);
        SpawnObjectAtGeoDelegate SpawnObjectAtGeoDelegateInstance = SpawnObjectOnGameThread;

        delegate bool DestroyObjectDelegate(string name);
        DestroyObjectDelegate DestroyObjectDelegateInstance = DestroyObject;

        delegate bool EnableWeatherVisualEffectsDelegate(bool status);
        EnableWeatherVisualEffectsDelegate EnableWeatherVisualEffectsDelegateInstance = EnableWeatherVisualEffects;

        delegate bool SetWeatherVisualEffectsParamDelegate(int weatherParam, float value);
        SetWeatherVisualEffectsParamDelegate SetWeatherVisualEffectsParamDelegateInstance = SetWeatherVisualEffectsParam;

        delegate bool ResetWeatherEffectsDelegate();
        ResetWeatherEffectsDelegate ResetWeatherEffectsDelegateInstance = ResetWeatherEffects;

        delegate bool SetSunPositionFromDateTimeDelegate(string dateTimeStr, string format, bool isDst);
        SetSunPositionFromDateTimeDelegate SetSunPositionFromDateTimeDelegateInstance = SetSunPositionFromDateTime;

        delegate bool SetCloudShadowStrengthDelegate(float strength);
        SetCloudShadowStrengthDelegate SetCloudShadowStrengthDelegateInstance = SetCloudShadowStrength;

        delegate bool SetSegmentationIDByNameDelegate(string meshName, int segId, bool isNameRegex, bool useOwnerName);
        SetSegmentationIDByNameDelegate SetSegmentationIDByNameDelegateInstance = SetSegmentationIDByName;

        private static WorldSimApi _instance;

        private Dictionary<string, GameObject> _spawnedObjects;
        private List<WeatherFX> _weatherFXInstances;

        private void RegisterServiceMethodsImpl()
        {
            // Hook up callbacks
            PInvokeWrapper.SetSpawnObjectCallback(
                Marshal.GetFunctionPointerForDelegate(SpawnObjectDelegateInstance));

            PInvokeWrapper.SetSpawnObjectAtGeoCallback(
                Marshal.GetFunctionPointerForDelegate(SpawnObjectAtGeoDelegateInstance));

            PInvokeWrapper.SetDestroyObjectCallback(
                Marshal.GetFunctionPointerForDelegate(DestroyObjectDelegateInstance));

            PInvokeWrapper.SetEnableWeatherVisualEffectsCallback(
                Marshal.GetFunctionPointerForDelegate(EnableWeatherVisualEffectsDelegateInstance));

            PInvokeWrapper.SetSetWeatherVisualEffectsParamCallback(
                Marshal.GetFunctionPointerForDelegate(SetWeatherVisualEffectsParamDelegateInstance));

            PInvokeWrapper.SetResetWeatherEffectsCallback(
                Marshal.GetFunctionPointerForDelegate(ResetWeatherEffectsDelegateInstance));

            PInvokeWrapper.SetSetSunPositionFromDateTimeCallback(
                Marshal.GetFunctionPointerForDelegate(SetSunPositionFromDateTimeDelegateInstance));

            PInvokeWrapper.SetSetCloudShadowStrengthCallback(
                Marshal.GetFunctionPointerForDelegate(SetCloudShadowStrengthDelegateInstance));

            PInvokeWrapper.SetSetSegmentationIDByNameCallback(
                Marshal.GetFunctionPointerForDelegate(SetSegmentationIDByNameDelegateInstance));
        }


        // Spawning assets methods //
        public static string SpawnObjectOnGameThread(string name, string assetPath,
            InteropPose pose, InteropVector3 scale)
        {
            GameThreadDispatcher.RunCommandOnGameThread(
                () => SpawnObject(name, assetPath, pose, scale),
                waitForCompletion: true);
            return name; // TODO: check for name collisions + generate unique name(?)
        }

        public static bool DestroyObject(string name)
        {
            if (!_instance._spawnedObjects.ContainsKey(name))
            {
                Debug.LogWarning($"Can't destroy [{name}]. It either doesn't exist or it wasn't spawned by client.");
                return false;
            }

            GameThreadDispatcher.RunCommandOnGameThread(
                () => Destroy(_instance._spawnedObjects[name]), waitForCompletion: true);
            return true;
        }

        private static void SpawnObject(string name, string assetPath,
            InteropPose pose, InteropVector3 scale) // TODO: enable physics?
        {
            var newObj = new GameObject(name);
            _instance._spawnedObjects[name] = newObj;

            /// TODO: move asset loading to UnityUtils (common with UnityRobotLink)
            // Trim any leading "/" in the path to work with the relative Resources/
            // Unity paths
            assetPath = assetPath.StartsWith("/")
                ? assetPath.Substring(1)
                : assetPath;
            var meshFilter = newObj.AddComponent<MeshFilter>();
            var loadedMesh = Resources.Load<Mesh>(assetPath);
            if (loadedMesh != null)
            {
                meshFilter.mesh = loadedMesh;
            }
            else
            {
                Debug.LogWarning($"Couldn't load asset [{assetPath}].");
            }

            // Load all of the mesh's materials into MeshRenderer
            var meshRenderer = newObj.AddComponent<MeshRenderer>();
            meshRenderer.materials = Resources.LoadAll<Material>(assetPath);

            // Add mesh collider
            // (Note: The asset also needs to be saved with "Read/Write Enabled"
            // set for collisions to work in stand-alone game binaries.)
            var collider = newObj.AddComponent<MeshCollider>();
            // Mesh collider must be set as convex = true to work with RigidBody that
            // has isKinematic = false
            collider.convex = true;
            collider.sharedMesh =  meshFilter.mesh;
            collider.enabled = true;

            newObj.transform.position = UnityTransform.NEDToUnityEUN(pose.position);
            newObj.transform.rotation = UnityTransform.NEDToUnityEUN(pose.orientation);
            newObj.transform.localScale = new Vector3(scale.y, scale.z, scale.x);
        }


        // Weather Effects methods //
        public static bool EnableWeatherVisualEffects(bool status)
        {
            GameThreadDispatcher.RunCommandOnGameThread(() =>
            {
                foreach (var weatherFX in _instance._weatherFXInstances)
                {
                    weatherFX.IsEnabled = status;
                    weatherFX.gameObject.SetActive(status);
                }
            }, waitForCompletion: true);
            return true;
        }

        public static bool SetWeatherVisualEffectsParam(int param, float value)
        {
            GameThreadDispatcher.RunCommandOnGameThread(() =>
            {
                var weatherParam = (WeatherParameter)param;
                if (weatherParam != WeatherParameter.Snow)
                {
                    Debug.LogWarning("Only snow effect supported.");
                }

                foreach (var weatherFX in _instance._weatherFXInstances)
                {
                    weatherFX.ParamScalars[weatherParam] = value;
                }
            }, waitForCompletion: true);
            return true;
        }

        public static bool ResetWeatherEffects()
        {
            GameThreadDispatcher.RunCommandOnGameThread(() =>
            {
                foreach (var weatherFX in _instance._weatherFXInstances)
                {
                    weatherFX.Reset();
                }
            }, waitForCompletion: true);
            return true;
        }

        public static bool SetSunPositionFromDateTime(string dateTimeStr, string format, bool isDst)
        {
            Debug.LogWarning("SetSunPositionFromDateTime not implemented.");
            return false;
        }

        public static bool SetCloudShadowStrength(float strength)
        {
            Debug.LogWarning("SetCloudShadowStrength not implemented.");
            return false;
        }


        // Segmentation methods //
        public static bool SetSegmentationIDByName(string meshName, int segId, bool isNameRegex, bool useOwnerName)
        {
            Debug.LogWarning("SetSegmentationIDByName not implemented.");
            return false;
        }

        public static void Initialize()
        {
            if (_instance == null)
            {
                _instance = new GameObject("WorldSimApi").AddComponent<WorldSimApi>();
                _instance.gameObject.hideFlags = HideFlags.HideAndDontSave;
                _instance._spawnedObjects = new Dictionary<string, GameObject>();
                _instance._weatherFXInstances = new List<WeatherFX>();
            }
        }

        public static void RegisterServiceMethods()
        {
            if (_instance == null)
            {
                Initialize();
            }

            // (Re)register service methods with the sim scene
            _instance.RegisterServiceMethodsImpl();
        }

        public static void Unload()
        {
            if (_instance != null)
            {
                foreach (var spawnedObj in _instance._spawnedObjects)
                {
                    Destroy(spawnedObj.Value);
                }
                _instance._spawnedObjects.Clear();

                foreach (var weatherFX in _instance._weatherFXInstances)
                {
                    Destroy(weatherFX);
                }
                _instance._weatherFXInstances.Clear();
            }
        }

        void OnDestroy()
        {
            Unload();
            Destroy(_instance.gameObject);
            _instance = null;
        }

        // Weather setup
        /// <summary>
        /// Attach an instance of WeatherFX to the vehicle.
        /// This is necessary because it would be computationally inefficient to render a weather particle system
        /// across a potentially very large environment. Instead, we render weather particles in the local area
        /// of each vehicle. In practice, this produces convincing weather effects because particles in the
        /// distance wouldn't be noticeable.
        /// </summary>
        /// <param name="robot">Game object of robot to which weather effects will be attached.</param>
        public static void AttachWeatherFXToRobot(GameObject robot)
        {
            var weatherGameObject = Resources.Load("WeatherFX/Prefabs/WeatherFX") as GameObject;

            weatherGameObject = GameObject.Instantiate(weatherGameObject, robot.transform);
            var weatherFX = weatherGameObject.GetComponent<WeatherFX>();
            weatherGameObject.SetActive(false);
            // TODO: hide the weather component from showing in the hierarchy?

            // Apply weather effect at absolute world scale regardless of the parent robot's scale.
            // Note that this won't work if the parent is unevenly stretched along different axes.
            Vector3 worldScale = weatherFX.transform.lossyScale;
            weatherFX.transform.localScale = Vector3.Scale(weatherFX.transform.localScale, new Vector3(1.0f / worldScale.x, 1.0f / worldScale.y, 1.0f / worldScale.z));

            _instance._weatherFXInstances.Add(weatherFX);
        }
    }
}
