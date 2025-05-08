// Copyright (C) Microsoft Corporation. All rights reserved.

// Script based on UnityGLTF plugin's GLTFComponent.cs. 
// Modified onLoadComplete action to contain GameObject info for post-load processing. 
// Removed unnecesary checks, added isLoadingComplete for tile management in GISRenderer.cs.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Runtime.ExceptionServices;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.Events;
using UnityGLTF.Loader;

namespace UnityGLTF
{
    public class KhronosGLTFComponent : MonoBehaviour
    {
        public string GLTFUri = null;
        public bool Multithreaded = true;
        public ImporterFactory Factory = null;
        public UnityAction<GameObject, ExceptionDispatchInfo> onLoadComplete;

        [SerializeField]
        private bool loadOnStart = true;

        [SerializeField] private int RetryCount = 10;
        [SerializeField] private float RetryTimeout = 2.0f;
        private int numRetries = 0;


        public int MaximumLod = 300;
        public int Timeout = 8;
        public GLTFSceneImporter.ColliderType Collider = GLTFSceneImporter.ColliderType.None;
        public GameObject LastLoadedScene { get; private set; } = null;

        [SerializeField]
        private Shader shaderOverride = null;

        public bool isLoadingComplete = false;

        private async void Start()
        {
            if (!loadOnStart) return;

            try
            {
                await Load();
            }
            catch (HttpRequestException)
            {
                if (numRetries++ >= RetryCount)
                    throw;

                Debug.LogWarning("Load failed, retrying");
                await Task.Delay((int)(RetryTimeout * 1000));
                Start();
            }
        }

        public async Task Load()
        {
            var importOptions = new ImportOptions
            {
                AsyncCoroutineHelper = gameObject.GetComponent<AsyncCoroutineHelper>() ?? gameObject.AddComponent<AsyncCoroutineHelper>()
            };

            GLTFSceneImporter sceneImporter = null;
            try
            {
                Factory = Factory ?? ScriptableObject.CreateInstance<DefaultImporterFactory>();

                string dir = URIHelper.GetDirectoryName(GLTFUri);
                importOptions.DataLoader = new UnityWebRequestLoader(dir);
                sceneImporter = Factory.CreateSceneImporter(
                    Path.GetFileName(GLTFUri),
                    importOptions
                );

                sceneImporter.SceneParent = gameObject.transform;
                sceneImporter.Collider = Collider;
                sceneImporter.MaximumLod = MaximumLod;
                sceneImporter.Timeout = Timeout;
                sceneImporter.IsMultithreaded = Multithreaded;
                sceneImporter.CustomShaderName = shaderOverride ? shaderOverride.name : null;

                await sceneImporter.LoadSceneAsync(onLoadComplete: LoadCompleteAction);

                if (shaderOverride != null)
                {
                    Renderer[] renderers = gameObject.GetComponentsInChildren<Renderer>();
                    foreach (Renderer renderer in renderers)
                    {
                        renderer.sharedMaterial.shader = shaderOverride;
                    }
                }

                LastLoadedScene = sceneImporter.LastLoadedScene;
            }
            finally
            {
                if (importOptions.DataLoader != null)
                {
                    sceneImporter?.Dispose();
                    sceneImporter = null;
                    importOptions.DataLoader = null;
                }
            }
        }

        private void LoadCompleteAction(GameObject obj, ExceptionDispatchInfo exceptionDispatchInfo)
        {
            isLoadingComplete = true;
            onLoadComplete?.Invoke(obj, exceptionDispatchInfo);
        }
    }
}
