// Copyright (C) Microsoft Corporation. All rights reserved.

using System.Collections;
using System.Collections.Generic;
using UnityProjectAirSim.Config;
using UnityEngine;

namespace UnityProjectAirSim.Robot
{
    //[RequireComponent(typeof(MeshRenderer), typeof(Collider))]
    public class UnityRobotLink : MonoBehaviour
    {
        private Vector3 _initialLocalPosition = new Vector3();
        private Quaternion _initialLocalRotation = new Quaternion();

        // Start is called before the first frame update
        void Start() { }

        // Update is called once per frame
        void Update() { }

        public void Initialize(LinkConfig linkConfig, bool IsRoot = false)
        {
            UnityUtils.InitializePose(gameObject, linkConfig.Visual?.Origin);
            _initialLocalPosition = transform.localPosition;
            _initialLocalRotation = transform.localRotation;

            InitializeGeometry(linkConfig, IsRoot);
        }

        private void InitializeGeometry(LinkConfig linkConfig, bool IsRoot)
        {
            // TODO: what if Geometry is null but collision isnt?
            if (linkConfig?.Visual?.Geometry == null ||
                linkConfig.Visual.Geometry.Name.Length == 0)
            {
                return;
            }

            // Trim any leading "/" in the path to work with the relative Resources/
            // Unity paths
            string assetPath = linkConfig.Visual.Geometry.Name.StartsWith("/")
                                   ? linkConfig.Visual.Geometry.Name.Substring(1)
                                   : linkConfig.Visual.Geometry.Name;
            var meshFilter = gameObject.AddComponent<MeshFilter>();
            meshFilter.mesh = Resources.Load<Mesh>(assetPath);

            // Load all of the mesh's materials into MeshRenderer
            var meshRenderer = gameObject.AddComponent<MeshRenderer>();
            meshRenderer.materials = Resources.LoadAll<Material>(assetPath);

            if (IsRoot && linkConfig.Collision !=
                              null)  // TODO set up defaults at config classes
            {
                InitializeCollider(linkConfig.Collision, meshFilter.mesh);
            }
        }

        private void InitializeCollider(CollisionConfig collisionConfig,
                                        Mesh mesh)
        {
            var collider = gameObject.AddComponent<MeshCollider>();

            // Mesh collider must be set as convex = true to work with RigidBody that
            // has isKinematic = false
            collider.convex = true;
            collider.sharedMesh = mesh;
            collider.enabled = collisionConfig.IsEnabled;

            // A RigidBody must also be attached to get collision detections from the
            // collider
            var rigidBody = gameObject.AddComponent<Rigidbody>();
            rigidBody.isKinematic =
                false;  // isKinematic must be false to get full collision info like
                        // normal and penetration
            rigidBody.useGravity = false;
            rigidBody.angularDrag = 0;
            rigidBody.collisionDetectionMode =
                CollisionDetectionMode.ContinuousDynamic;
            // TODO: I'm missing some collision settings from UE
        }

        public void SetActuatedTransform(InteropActuatedTransform actuatedTransform)
        {
            switch (actuatedTransform.apply_order)
            {
                // TODO Support Pre and Post apply orders
                case ApplyOrder.Pre:
                    Debug.Log("WARNING: Unsupported ApplyOrder.Pre transformation order.");
                    break;

                case ApplyOrder.PreTranslation:
                    transform.localPosition = _initialLocalPosition + UnityTransform.NEDToUnityEUN(actuatedTransform.actuated_transform.position);
                    transform.localRotation = _initialLocalRotation * UnityTransform.NEDToUnityEUN(actuatedTransform.actuated_transform.orientation);
                    break;

                case ApplyOrder.Post:
                    Debug.Log("WARNING: Unsupported ApplyOrder.Post transformation order.");
                    break;
            }
        }
    }
}
