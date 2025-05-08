using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.ExceptionServices;
using System.Threading.Tasks;

using GLTF;
using GLTF.Schema;
using UnityGLTF;
using UnityGLTF.Loader;
using UnityEngine;
using Unity.Jobs;
using Unity.Collections;

namespace UnityProjectAirSim.Rendering.Scene
{
    public class GltfDataProvider
    {
        private readonly IDataLoader _fileLoader;
        private readonly string _tileDir;
        private readonly Vector3 _homeInEcef;
        private readonly Matrix4x4 _ecefToNeuRotMat;
        private readonly GameObject _gltfPrefab;

        public GltfDataProvider(string tilesDir)
        {
            _tileDir = tilesDir;
            _fileLoader = new FileLoader(tilesDir);
            PInvokeWrapper.GetHomeGeoPointInECEF(
                out var x, out var y, out var z);
            _homeInEcef = new Vector3((float)x, (float)y, (float)z);
            _ecefToNeuRotMat = GetEcefToNeuMatrix();

            // Find GLTF prefab in Resources
            _gltfPrefab = (GameObject)Resources.Load("Prefabs/GISTileGLTF");
        }

        private Matrix4x4 GetEcefToNeuMatrix()
        {
            var startPtr = PInvokeWrapper.GetEcefToNeuRotationMatrix();
            double[] matArray = new double[9];
            Marshal.Copy(startPtr, matArray, 0, 9);
            Marshal.FreeCoTaskMem(startPtr);

            var mat = new Matrix4x4(); // no support for 3x3 mat
            for (int colIdx = 0; colIdx < 3; ++colIdx)
            {
                var col = new Vector4();
                for (int rowIdx = 0; rowIdx < 3; ++rowIdx)
                {
                    int i = colIdx * 3 + rowIdx;
                    col[rowIdx] = (float)matArray[i];
                }
                col[3] = 0;
                mat.SetColumn(colIdx, col);
            }

            // not adding home ECEF offset because translation needs to happen before rotation.
            // 4x4 matrix is filled as:
            // [ R00, R01, R02, translation.x ]
            // [ R10, R11, R12, translation.y ]
            // [ R20, R21, R22, translation.z ]
            // [   0,   0,   0,             1 ]
            mat.SetColumn(3, new Vector4(0, 0, 0, 1));

            if (!mat.ValidTRS())
            {
                Debug.LogWarning("ECEF matrix not a valid transform");
            }
            return mat;
        }

        public GameObject LoadBingTile(TileKey tileKey)
        {
            string quadKey = TileKeyToQuadkey(tileKey);
            string tileFileName = quadKey + ".glb";
            string filePath = Path.Combine(_tileDir, tileFileName);

            if (!File.Exists(filePath)) 
                return null;

            // Create GLTF(modified) prefab, like in khronos samples
            // Loads file on gameObject Start
            GameObject tileObj = GameObject.Instantiate(_gltfPrefab);
            tileObj.GetComponent<KhronosGLTFComponent>().GLTFUri = new Uri(filePath).AbsoluteUri;
            tileObj.GetComponent<KhronosGLTFComponent>().onLoadComplete += OnTileLoadComplete;
            tileObj.name = $"Tile-{quadKey}";

            return tileObj;
        }

        private void OnTileLoadComplete(GameObject obj, ExceptionDispatchInfo exceptionDispatchInfo)
        {
            obj.GetComponentInParent<AsyncCoroutineHelper>().enabled = false;
            obj.transform.GetChild(0).rotation = Quaternion.identity;
            TransformVertices(obj);
        }

        private void TransformVertices(GameObject gameObj)
        {
            var meshFilter = gameObj.GetComponentInChildren<MeshFilter>();
            if (meshFilter == null)
            {
                Debug.LogWarning("No mesh filter in tile");
                return;
            }

            NativeArray<Vector3> inputArray = new NativeArray<Vector3>(meshFilter.mesh.vertices, Allocator.TempJob);
            NativeArray<Vector3> outputArray = new NativeArray<Vector3>(meshFilter.mesh.vertexCount, Allocator.TempJob);

            TransformVerticiesJob addJob = new TransformVerticiesJob
            {
                inputArray = inputArray,
                outputArray = outputArray,
                homeInEcef = _homeInEcef,
                ecefToNeuRotMat = _ecefToNeuRotMat
            };

            JobHandle jobHandle = addJob.Schedule(meshFilter.mesh.vertexCount, 1);
            jobHandle.Complete();

            Vector3[] transformedVerticies = new Vector3[outputArray.Length];
            outputArray.CopyTo(transformedVerticies);
            meshFilter.mesh.vertices = transformedVerticies;

            inputArray.Dispose();
            outputArray.Dispose();
            
            meshFilter.mesh.RecalculateBounds();

            var meshConvexCollider = gameObj.AddComponent<MeshCollider>();
            meshConvexCollider.sharedMesh = meshFilter.mesh;
            meshConvexCollider.convex = true;
        }

        private struct TransformVerticiesJob : IJobParallelFor
        {
            public NativeArray<Vector3> inputArray;
            public NativeArray<Vector3> outputArray;
            public Vector3 homeInEcef;
            public Matrix4x4 ecefToNeuRotMat;

            public void Execute(int index)
            {
                var ecefPos = inputArray[index];
                ecefPos.x = -ecefPos.x; // Importer flipped from GLTF right-hand coord system to Unity left-handed
                ecefPos = ecefPos - homeInEcef;
                var neu = ecefToNeuRotMat.MultiplyVector(ecefPos);
                // NEU -> Unity EUN
                outputArray[index] = new Vector3((float)neu.y, (float)neu.z, (float)neu.x);
            }
        }

        // Copied from BingMapsUtils::TileXYToQuadKey
        private static string TileKeyToQuadkey(TileKey tileKey)
        {
            string quadkey = "";
            for (int i_lod = tileKey.lod; i_lod > 0; --i_lod)
            {
                int digit = 0;
                int mask = 1 << (i_lod - 1);
                if ((tileKey.x & mask) != 0) digit += 1;
                if ((tileKey.y & mask) != 0) digit += 2;
                quadkey += digit.ToString();
            }

            return quadkey;
        }
    }
}
