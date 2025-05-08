using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

using UnityEngine;

namespace UnityProjectAirSim
{
    public class GameThreadDispatcher : MonoBehaviour
    {
        private static GameThreadDispatcher _instance = null;
        private static readonly Queue<Task> _taskQueue = new Queue<Task>();
        private static readonly Thread _mainThread = Thread.CurrentThread;

        public static void RunCommandOnGameThread(Action action, bool waitForCompletion = false)
        {
            if (IsInGameThread())
            {
                action();
            }
            else
            {
                var task = new Task(action); // TODO: or StartCoroutine(action)?
                lock (_taskQueue)
                {
                    _taskQueue.Enqueue(task);
                }

                if (waitForCompletion)
                {
                    task.Wait();
                }
            }
        }

        public static bool IsInGameThread()
        {
            return Thread.CurrentThread == _mainThread;
        }

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
        private static void Initialize()
        {
            if (_instance == null)
            {
                _instance = new GameObject("GameThreadDispatcher").AddComponent<GameThreadDispatcher>();
                _instance.gameObject.hideFlags = HideFlags.HideAndDontSave;
                DontDestroyOnLoad(_instance.gameObject);
            }
        }

        private void Update()
        {
            lock (_taskQueue)
            {
                // TODO: handle case where queue is drained slower than filled:
                // latch current queue count and stop after dequeueing that
                // amount.
                while (_taskQueue.Count > 0)
                {
                    _taskQueue.Dequeue().RunSynchronously();
                }
            }
        }

        void OnDestroy()
        {
            Destroy(_instance.gameObject);
            _instance = null;
        }
    }

}