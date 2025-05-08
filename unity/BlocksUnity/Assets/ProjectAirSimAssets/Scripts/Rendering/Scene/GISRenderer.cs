using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Runtime.InteropServices;

using UnityEngine;
using System.Threading.Tasks;
using UnityGLTF;

namespace UnityProjectAirSim.Rendering.Scene
{
    public class GISRenderer : MonoBehaviour
    {
        private GltfDataProvider _tileProvider;
        private List<GameObject> _robots;
        private List<Vector3> _lastRobotPos;
        private WaitForEndOfFrame _waitForEndOfFrame;
        private bool _renderGISTiles = true;

        private readonly Dictionary<TileKey, GameObject> _renderedTiles = new Dictionary<TileKey, GameObject>();
        private SemaphoreSlim _renderedTilesLock = new SemaphoreSlim(1, 1);
        private HashSet<Task<GameObject>> _tileLoadTasks = new HashSet<Task<GameObject>>();
        private Dictionary<int, TileKey> _taskIdToTileKey = new Dictionary<int, TileKey>();
        private List<TileKey> _tileKeysToRender = new List<TileKey>();
        private int numTilesPerFrame = 3;

        void Start()
        {

        }

        public void Initialize(string tilesDir, List<GameObject> robots)
        {
            if (!Directory.Exists(tilesDir))
            {
                // throw new Exception($"Tile directory [{tilesDir}] doesn't exist");

                // Just log a warning for now, since throwing an exception here
                // breaks the current scene reloading loop until it can be
                // updated to a proper RunCommandOnGameThread() call.
                Debug.Log("WARNING: GIS tile directory [" + tilesDir + "] doesn't exist.");
                return;
            }

            _tileProvider = new GltfDataProvider(tilesDir);
            _robots = robots;
            _waitForEndOfFrame = new WaitForEndOfFrame();

            _lastRobotPos = new List<Vector3>(robots.Count);
            foreach (var robot in robots)
            {
                _lastRobotPos.Add(robot.transform.position);
            }
            StartCoroutine(LoadGISRoutine());
        }

        // Unused or deprecated
        public async Task AddTileGameObject(TileKey tileKey, Task<GameObject> tileObjTask)
        {
            await _renderedTilesLock.WaitAsync();
            _renderedTiles.Add(tileKey, tileObjTask.Result);
            _renderedTilesLock.Release();
        }

        // Unused or deprecated
        public async Task RemoveTileGameObject(TileKey tileKey)
        {
            await _renderedTilesLock.WaitAsync();
            _renderedTiles.Remove(tileKey);
            _renderedTilesLock.Release();
        }

        public async Task GetTilesToRender()
        {
            var robotsMoved = false;
            for (int robotIdx = 0; robotIdx < _robots.Count; ++robotIdx)
            {
                var currRobotPos = _robots[robotIdx].transform.position;
                var dist = currRobotPos - _lastRobotPos[robotIdx];

                if (dist.sqrMagnitude > 6.0)
                {
                    robotsMoved = true;
                    _lastRobotPos[robotIdx] = currRobotPos;
                }
            }

            if (!robotsMoved && _renderedTiles.Any())
            {
                return;
            }

            int lenTileKeysBuffer = 0;
            var tileKeyBuffer = await Task.Run(() => PInvokeWrapper.GetTileKeysToRender(out lenTileKeysBuffer));
            var tileKeysToRender = new List<TileKey>();
            if (lenTileKeysBuffer > 0)
            {
                int[] flatTiles = new int[lenTileKeysBuffer];
                Marshal.Copy(tileKeyBuffer, flatTiles, 0, lenTileKeysBuffer);
                Marshal.FreeCoTaskMem(tileKeyBuffer);
                for (int i = 0; i < lenTileKeysBuffer;)
                {
                    var tileKey = new TileKey(flatTiles[i++], flatTiles[i++], flatTiles[i++]);
                    tileKeysToRender.Add(tileKey);
                }
            }

            _tileKeysToRender = tileKeysToRender;
        }

        private IEnumerator LoadGISRoutine()
        {
            while (_renderGISTiles)
            {
                if (_tileProvider == null)
                {
                    yield return _waitForEndOfFrame;
                }

                yield return GetTilesToRender();


                for (int i = 0; i < _tileKeysToRender.Count; i++)
                {
                    if (!_renderedTiles.ContainsKey(_tileKeysToRender[i]))
                    {
                        GameObject tempTile = _tileProvider.LoadBingTile(_tileKeysToRender[i]);
                        if (tempTile != null)
                        {
                            _renderedTiles.Add(_tileKeysToRender[i], tempTile);
                            
                            // Yield every Nth tile (1 - smoother but slower, 10 - lags but faster)
                            if(i % numTilesPerFrame == 0)
                                yield return _waitForEndOfFrame;
                        }
                    }
                }

                var tileKeysToDestroy = _renderedTiles.Keys.Except(_tileKeysToRender);
                foreach (var tileKey in tileKeysToDestroy.Reverse())
                {
                    var tileObj = _renderedTiles[tileKey];
                    if (tileObj.GetComponent<KhronosGLTFComponent>().isLoadingComplete)
                    {
                        _renderedTiles.Remove(tileKey);
                        Destroy(tileObj);
                    }
                }

                yield return _waitForEndOfFrame;
            }
        }

        public void Unload()
        {
            _renderGISTiles = false;

            foreach (var tileKey in _renderedTiles.Keys)
            {
                var tileObj = _renderedTiles[tileKey];
                Destroy(tileObj);
            }
            _renderedTiles.Clear();
        }

        void OnDestroy()
        {
            Unload();
        }
    }
}