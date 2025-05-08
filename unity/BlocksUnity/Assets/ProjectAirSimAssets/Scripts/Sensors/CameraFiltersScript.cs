// Copyright (C) Microsoft Corporation. All rights reserved.

using System.Collections.Generic;
using System.Text.RegularExpressions;
using UnityProjectAirSim;
using UnityProjectAirSim.Config;
using UnityEngine;
using UnityEngine.Rendering;

namespace UnityProjectAirSim.Sensors
{
    /*
     * MonoBehaviour class that is attached to cameras in the scene.
     * Used for applying image filters based on settings.json or Image request by
     * the client to record the data. The three filters, Depth, Segment and Vision
     * are supported as in Unreal. Note : Most of the code is based on Unity's
     * Image Synthesis project
     */

    public class CameraFiltersScript : MonoBehaviour
    {
        public ImageType effect;
        public Shader effectsShader;

        public static Dictionary<string, int> segmentationIds =
            new Dictionary<string, int>();

        private void Start()
        {
            if (!effectsShader) effectsShader = Shader.Find("Hidden/CameraEffects");
            var renderers = FindObjectsOfType<Renderer>();
            var mpb = new MaterialPropertyBlock();
            foreach (var r in renderers)
            {
                var id = r.gameObject.GetInstanceID();
                var layer = r.gameObject.layer;

                mpb.SetColor("_ObjectColor", ColorEncoding.EncodeIDAsColor(id));
                mpb.SetColor("_CategoryColor", ColorEncoding.EncodeLayerAsColor(layer));
                r.SetPropertyBlock(mpb);
            }
        }

        public void SetShaderEffect(Camera camera, ImageType type)
        {
            effect = type;
            UpdateCameraEffect(camera);
        }

        public static void InitializeSegmentationIdsDict()
        {
            // Initialize segmentation IDs as -1 to indicate un-set
            segmentationIds.Clear();
            var renderers = FindObjectsOfType<Renderer>();
            foreach (var r in renderers)
            {
                // Note: There could be multiple renderers for the same
                // GameObject name, so just add once to the map since they
                // represent the same segmentation object.
                if (!segmentationIds.ContainsKey(r.gameObject.name))
                {
                    segmentationIds.Add(r.gameObject.name, -1);
                }
            }
        }

        public static bool SetSegmentationId(string objectName, int segmentationId,
                                             bool isNameRegex)
        {
            List<string> keyList = new List<string>(segmentationIds.Keys);
            if (isNameRegex)
            {
                bool isValueSet = false;
                foreach (string s in keyList)
                {
                    // TODO Regex doesn't match for names with parentheses,
                    // need to fix handling these
                    if (!Regex.IsMatch(s, objectName))
                    {
                        continue;
                    }
                    segmentationIds[s] = segmentationId;
                    isValueSet = true;
                }
                return isValueSet;
            }

            if (!segmentationIds.ContainsKey(objectName))
            {
                return false;
            }
            segmentationIds[objectName] = segmentationId;
            return true;
        }

        public static int GetSegmentationId(string objectName)
        {
            if (segmentationIds.ContainsKey(objectName))
            {
                return segmentationIds[objectName];
            }
            return -1;
        }

        public static int CalculateSegmentationId(string objectName)
        {
            var segName = objectName.ToLower();
            // Calculate a segmentation ID from a hash for each mesh's base name chars
            int segId = 5;
            for (int idx = 0; idx < segName.Length; ++idx)
            {
                var char_num = (int)segName[idx];
                if (char_num < 97) continue;  // ignore numerics and other punctuations
                segId += char_num;
            }
            segId %= 256;
            return segId;
        }

        private void SetSegmentationEffect(Camera camera)
        {
            var renderers = FindObjectsOfType<Renderer>();
            var mpb = new MaterialPropertyBlock();
            foreach (var r in renderers)
            {
                var id = r.gameObject.GetInstanceID();
                segmentationIds.TryGetValue(r.gameObject.name, out id);
                var layer = r.gameObject.layer;

                mpb.SetColor("_ObjectColor", ColorEncoding.EncodeIDAsColor(id));
                mpb.SetColor("_CategoryColor", ColorEncoding.EncodeLayerAsColor(layer));
                r.SetPropertyBlock(mpb);
            }

            camera.renderingPath = RenderingPath.Forward;
            SetupCameraWithReplacementShader(camera, 0, Color.gray);
        }

        private void SetDepthPerspectiveEffect(Camera camera)
        {
            camera.renderingPath = RenderingPath.Forward;
            SetupCameraWithReplacementShader(camera, 2, Color.white);
        }

        private void ResetCameraEffects(Camera camera)
        {
            camera.renderingPath = RenderingPath.UsePlayerSettings;
            camera.clearFlags = CameraClearFlags.Skybox;
            camera.SetReplacementShader(null, null);
        }

        private void SetupCameraWithReplacementShader(Camera camera, int mode,
                                                      Color clearColor)
        {
            var cb = new CommandBuffer();
            cb.SetGlobalFloat("_OutputMode", (int)mode);
            camera.AddCommandBuffer(CameraEvent.BeforeForwardOpaque, cb);
            camera.AddCommandBuffer(CameraEvent.BeforeFinalPass, cb);
            camera.SetReplacementShader(effectsShader, "");
            camera.backgroundColor = clearColor;
            camera.clearFlags = CameraClearFlags.SolidColor;
        }

        private void UpdateCameraEffect(Camera camera)
        {
            if (!camera)
            {
                return;
            }
            camera.RemoveAllCommandBuffers();
            switch (effect)
            {
                case ImageType.Scene:
                    ResetCameraEffects(camera);
                    break;

                case ImageType.DepthPerspective:
                    SetDepthPerspectiveEffect(camera);
                    break;

                case ImageType.DepthPlanar:  // TODO
                    Debug.LogWarning("DepthPlanar not yet implemented.");
                    break;

                case ImageType.Segmentation:
                    SetSegmentationEffect(camera);
                    break;

                default:
                    ResetCameraEffects(camera);
                    break;
            }
        }
    }
}