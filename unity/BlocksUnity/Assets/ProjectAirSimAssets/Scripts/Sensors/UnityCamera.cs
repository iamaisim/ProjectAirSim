// Copyright (C) Microsoft Corporation. All rights reserved.

using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using Unity.Collections;
using UnityProjectAirSim;
using UnityProjectAirSim.Config;
using UnityEngine;
using UnityEngine.Rendering;
using System.Runtime.InteropServices;

namespace UnityProjectAirSim.Sensors
{
    [RequireComponent(typeof(Camera))]
    public class UnityCamera : UnitySensor
    {
        private int _simRobotIndex = -1;
        private int _simSensorIndex = -1;
        private Camera _attachedCamera;
        private CameraFiltersScript _shaderScript;

        private WaitForSeconds _waitUntilNext;
        private WaitForEndOfFrame _waitForEndOfFrame;

        private Camera[] _captureCams = new Camera[(int)ImageType.Count];
        private RenderTexture[] _renderTextures =
            new RenderTexture[(int)ImageType.Count];

        private Queue<ValueTuple<Thread, ImagePackingTaskAsync>> _imagePackingTasks =
            new Queue<ValueTuple<Thread, ImagePackingTaskAsync>>();

        private static bool IsDepthImage(CameraCaptureConfig cs) =>
                cs.ImageType == ImageType.DepthPerspective ||
                cs.ImageType == ImageType.DepthPlanar;

        // Start is called before the first frame update
        void Start() { }

        // Update is called once per frame
        void Update()
        {
            IntPtr strPtr = PInvokeWrapper.CameraGetLookAtObject(_simRobotIndex, _simSensorIndex);
            string lookAtObjectName = Marshal.PtrToStringAuto(strPtr);
            if (lookAtObjectName.Length > 0)
            {
                GameObject lookAtObject = GameObject.Find(lookAtObjectName);
                if (lookAtObject)
                {
                    Debug.Log(lookAtObject);
                    this.transform.LookAt(lookAtObject.transform.position);
                }
                else
                {
                    Debug.LogWarning(string.Format("[UnityCamera] Cannot look at '%hs' because it is not in the scene!", lookAtObjectName));
                    PInvokeWrapper.CameraResetPose(_simRobotIndex, _simSensorIndex, false);
                }
                PInvokeWrapper.CameraMarkPoseUpdateAsCompleted(_simRobotIndex, _simSensorIndex);
            }
            else if (PInvokeWrapper.CameraIsPoseUpdatePending(_simRobotIndex, _simSensorIndex))
            {
                InteropPose newPose = PInvokeWrapper.CameraGetDesiredPose(_simRobotIndex, _simSensorIndex);
                UnityUtils.InitializePose(gameObject, newPose);
                PInvokeWrapper.CameraMarkPoseUpdateAsCompleted(_simRobotIndex, _simSensorIndex);
            }

            // TODO
            //if (SimCamera.IsSettingsUpdatePending())
            //{
            //    UpdateCameraSettings();
            //    SimCamera.MarkSettingsUpdateAsCompleted();
            //}
        }

        public void Initialize(int robotIndex, CameraConfig camSettings)
        {
            _simRobotIndex = robotIndex;
            _simSensorIndex =
                PInvokeWrapper.GetSensorIndex(robotIndex, camSettings.Id);
            _attachedCamera = GetComponent<Camera>();
            _shaderScript = gameObject.AddComponent<CameraFiltersScript>();
            _waitUntilNext = new WaitForSeconds(camSettings.CaptureInterval);
            _waitForEndOfFrame = new WaitForEndOfFrame();

            if (!camSettings.IsEnabled)
            {
                _attachedCamera.enabled = false;
                return;
            }

            foreach (var captureSettings in camSettings.CaptureSettings)
            {
                InitializeCapture(captureSettings);  // TODO: add other image types
            }

            if (camSettings.CaptureSettings.Any(c => c.CaptureEnabled))
            {
                StartCoroutine(CaptureFramesRoutine(camSettings));
            }
        }

        private void InitializeCapture(CameraCaptureConfig captureSettings)
        {
            _attachedCamera.enabled = true;
            _attachedCamera.nearClipPlane =
                0.01f;  // TODO Confirm if this matches Unreal default
            _attachedCamera.farClipPlane =
                10000.0f;  // TODO Confirm if this matches Unreal default

            // Render captures after main Unity camera (higher depth renders later)
            _attachedCamera.depth = Camera.main.depth + 1;

            if (captureSettings.StreamingEnabled)
            {
                _attachedCamera.tag = "MainCamera";
                // Render streaming cams after non-streaming cams to show as main
                // viewport
                _attachedCamera.depth += 1;
            }

            if (captureSettings.CaptureEnabled)
            {
                // TODO: does the rendertexture format need to match the Texture2D
                // format?
                RenderTexture renderTexture;
                if (IsDepthImage(captureSettings))
                {
                    renderTexture =
                        new RenderTexture(captureSettings.Width, captureSettings.Height,
                                          24, RenderTextureFormat.RFloat);
                }
                else if (captureSettings.PixelsAsFloat)
                {
                    renderTexture =
                        new RenderTexture(captureSettings.Width, captureSettings.Height,
                                          24, RenderTextureFormat.ARGBFloat);
                }
                else
                {
                    renderTexture =
                        new RenderTexture(captureSettings.Width, captureSettings.Height,
                                          24 /*16*/, RenderTextureFormat.ARGB32);
                }
                renderTexture.Create();

                _attachedCamera.fieldOfView =
                    captureSettings.FovDegrees;  // TODO: shouldnt this be per camera?
                                                 // TODO: handle other settings: gamma, etc

                _renderTextures[(int)captureSettings.ImageType] = renderTexture;
                _captureCams[(int)captureSettings.ImageType]
                = CreateHiddenCameraCopy(captureSettings);
            }
        }

        private Camera CreateHiddenCameraCopy(CameraCaptureConfig captureSettings)
        {
            var camObj =
                new GameObject(captureSettings.ImageType.ToString(), typeof(Camera));
            camObj.hideFlags = HideFlags.HideAndDontSave;
            camObj.transform.parent = transform;

            var newCamera = camObj.GetComponent<Camera>();
            newCamera.CopyFrom(_attachedCamera);
            newCamera.targetTexture =
                _renderTextures[(int)captureSettings.ImageType];
            return newCamera;
        }

        private Texture2D CopyRenderTextureToCPU(TextureFormat format,
                                                 Rect captureRect)
        {
            // Note: Texture2D allocation is not automatically garbage
            // collected, needs to be manually destroyed when done using it
            // to prevent memory leak.
            var cpuTexture = new Texture2D((int)captureRect.width,
                                           (int)captureRect.height, format, false);
            cpuTexture.ReadPixels(captureRect, 0, 0);
            return cpuTexture;
        }

        private void CaptureSingleFrame(CameraCaptureConfig captureSettings,
                                        ref ImagePackingTaskAsync
                                            imagePackingTask)
        {
            if (!captureSettings.CaptureEnabled)
            {
                return;
            }

            var captureCam = _captureCams[(int)captureSettings.ImageType];
            var renderTexture = _renderTextures[(int)captureSettings.ImageType];

            _shaderScript.SetShaderEffect(captureCam, captureSettings.ImageType);

            var prevActiveRT = RenderTexture.active;
            var prevCameraRT = captureCam.targetTexture;

            // render to offscreen texture (readonly from CPU side)
            RenderTexture.active = renderTexture;
            captureCam.targetTexture = renderTexture;

            captureCam.Render();

            var captureRect =
                new Rect(0, 0, captureSettings.Width, captureSettings.Height);
            var captureResults = new CaptureResults { Settings = captureSettings };

            if (IsDepthImage(captureSettings))
            {
                // Even though the render texture's format for the depth image is RFloat,
                // we use RGBAFloat so that it gets read correctly into the Color struct.
                captureResults.Texture =
                    CopyRenderTextureToCPU(TextureFormat.RGBAFloat, captureRect);
                captureResults.ImageDataForFloat = captureResults.Texture.GetPixelData<Color>(0);
            }
            else if (!captureSettings.Compress && !captureSettings.PixelsAsFloat)
            {
                captureResults.Texture =
                    CopyRenderTextureToCPU(TextureFormat.RGBA32, captureRect);
                captureResults.ImageData = captureResults.Texture.GetPixelData<Color32>(0);
            }
            else if (captureSettings.Compress && !captureSettings.PixelsAsFloat)
            {
                captureResults.Texture =
                    CopyRenderTextureToCPU(TextureFormat.RGBA32, captureRect);
                captureResults.PngImageData =
                    captureResults.Texture.EncodeToPNG();  // Basically no background work here.
            }
            else if (!captureSettings.Compress && captureSettings.PixelsAsFloat)
            {
                captureResults.Texture =
                    CopyRenderTextureToCPU(TextureFormat.RGBAFloat, captureRect);
                captureResults.ImageDataForFloat = captureResults.Texture.GetPixelData<Color>(0);
            }

            imagePackingTask.CaptureResults[(int)captureSettings.ImageType] =
                captureResults;

            // restore state and cleanup
            captureCam.targetTexture = prevCameraRT;
            RenderTexture.active = prevActiveRT;

            // TODO: do we need to destroy the cpu texture when finished?
        }

        private IEnumerator CaptureFramesRoutine(CameraConfig camSettings)
        {
            while (_attachedCamera.enabled)
            {
                yield return _waitForEndOfFrame;

                // Clean up completed imageProcessingTask threads by
                // destroying their corresponding Texture2D allocations (must
                // be done on the main thread)
                while (_imagePackingTasks.Count > 0 &&
                    !_imagePackingTasks.Peek().Item1.IsAlive)
                {
                    var completedTask = _imagePackingTasks.Dequeue();
                    foreach (var captureResult in completedTask.Item2.CaptureResults)
                    {
                        if (captureResult != null)
                        {
                            Destroy(captureResult.Texture);
                        }
                    }
                }

                // Start a new image capture task unless the current capture
                // queue is already full
                if (!PInvokeWrapper.IsCameraCaptureQueueFull(_simRobotIndex,
                                                             _simSensorIndex))
                {
                    PInvokeWrapper.AddCameraRequestToCaptureQueue(
                        _simRobotIndex, _simSensorIndex, PoseUpdatedTimeStamp);

                    var imagePackingTask = new ImagePackingTaskAsync
                    {
                        // Settings general to the whole pack
                        Timestamp = PoseUpdatedTimeStamp,
                        CamPos = _attachedCamera.transform.position,
                        CamRot = _attachedCamera.transform.rotation,
                        SimRobotIndex = _simRobotIndex,
                        SimSensorIndex = _simSensorIndex,
                    };

                    foreach (var captureSettings in camSettings.CaptureSettings)
                    {
                        CaptureSingleFrame(captureSettings, ref imagePackingTask);
                    }

                    var imagePackingThread =
                        new Thread(new ThreadStart(imagePackingTask.PackImages));
                    imagePackingThread.Priority =
                        System.Threading.ThreadPriority
                            .BelowNormal;  // TODO: is this right?

                    _imagePackingTasks.Enqueue((imagePackingThread, imagePackingTask));
                    imagePackingThread.Start();

                    yield return _waitUntilNext;
                }
            }
        }

        private class CaptureResults
        {
            public CameraCaptureConfig Settings;
            public Texture2D Texture;

            // Only one of the following would be filled depending on the image type
            public NativeArray<Color32>
                ImageData;  // TODO: can use ReadOnly type instead
            public NativeArray<Color> ImageDataForFloat;
            public byte[] PngImageData;
        }

        /// <summary>
        /// Class to encapsulate data needed to spawn off background thread
        /// that packs images and publish them back to the client.
        /// </summary>
        private class ImagePackingTaskAsync
        {
            public CaptureResults[] CaptureResults =
                new CaptureResults[(int)ImageType.Count];
            public Int64 Timestamp;
            public Vector3 CamPos;
            public Quaternion CamRot;
            public int SimRobotIndex;
            public int SimSensorIndex;

            private InteropImageMessage PackSingleCapture(CaptureResults capture)
            {
                byte[] outImageData = new byte[] { };
                var encoding = "BGR";

                if (IsDepthImage(capture.Settings) && !capture.Settings.Compress)
                {
                    var outImageDataShort = new NativeArray<UInt16>(capture.ImageDataForFloat.Length, Allocator.Persistent);
                    int dataIndex = 0;
                    float depthMilliMax = (float)UInt16.MaxValue;
                    for (int y = capture.Settings.Height - 1; y >= 0; --y)
                    {
                        for (int x = 0; x < capture.Settings.Width; ++x)
                        {
                            var i = y * capture.Settings.Width + x;
                            var depthMeters = capture.ImageDataForFloat[i].r;
                            var depthMilli = (float)(depthMeters) * 1000F;
                            depthMilli = depthMilli > depthMilliMax ? depthMilliMax : depthMilli;
                            outImageDataShort[dataIndex++] = (UInt16)depthMilli;
                        }
                    }
                    outImageData = outImageDataShort.Reinterpret<byte>(sizeof(UInt16)).ToArray();
                    outImageDataShort.Dispose();
                    encoding = "16UC1"; // 16-bit unsigned, 1 channel for depth in mm
                }
                else if (!capture.Settings.Compress &&
                         !capture.Settings.PixelsAsFloat)
                {
                    outImageData = new byte[capture.ImageData.Length * 3];
                    int dataIndex = 0;
                    for (int y = capture.Settings.Height - 1; y >= 0; --y)
                    {
                        for (int x = 0; x < capture.Settings.Width; ++x)
                        {
                            var i = y * capture.Settings.Width + x;
                            var c = capture.ImageData[i];
                            outImageData[dataIndex++] = c.b;
                            outImageData[dataIndex++] = c.g;
                            outImageData[dataIndex++] = c.r;
                        }
                    }
                    encoding = "BGR";
                }
                else if (capture.Settings.Compress &&
                         !capture.Settings.PixelsAsFloat)
                {
                    outImageData = capture.PngImageData;
                    encoding = "PNG";
                }
                else if (!capture.Settings.Compress &&
                         capture.Settings.PixelsAsFloat)
                {
                    outImageData = new byte[capture.ImageDataForFloat.Length * 3];
                    int dataIndex = 0;
                    for (int y = capture.Settings.Height - 1; y >= 0; --y)
                    {
                        for (int x = 0; x < capture.Settings.Width; ++x)
                        {
                            var i = y * capture.Settings.Width + x;
                            var c = capture.ImageData[i];
                            outImageData[dataIndex++] = (byte)(c.b * 255);
                            outImageData[dataIndex++] = (byte)(c.g * 255);
                            outImageData[dataIndex++] = (byte)(c.r * 255);
                        }
                    }
                    encoding = "BGR";
                }
                else
                {
                    Console.WriteLine($"Unsupported combination of camera options.");
                }

                return new InteropImageMessage(
                    Timestamp, (uint)capture.Settings.Height,
                    (uint)capture.Settings.Width, encoding, big_endian
                    : false, step: 1, outImageData,
                      UnityTransform.UnityEUNToNED(CamPos),
                      UnityTransform.UnityEUNToNED(CamRot));
            }

            public void PackImages()
            {
                var imageMessages = new InteropImageMessage[(int)ImageType.Count];

                foreach (var capture in CaptureResults)
                {
                    if (capture != null)
                    {
                        var imageType = (int)capture.Settings.ImageType;
                        imageMessages[imageType]
                        = PackSingleCapture(capture);
                    }
                }

                PInvokeWrapper.PublishImages(SimRobotIndex, SimSensorIndex,
                                             imageMessages);
            }
        }
    }
}