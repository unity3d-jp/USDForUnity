using System;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(UsdMeshComponent))]
    public class UsdMeshEditor : UsdIComponentEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();

            var t = target as UsdMeshComponent;
            if(!t.schema.isInstance && !t.schema.isMaster && !t.schema.isInMaster)
            {
                EditorGUILayout.Space();
                if (GUILayout.Button("Precompute Normals / Tangents"))
                {
                    UsdPrecomputeNormalsWindow.Open(t.schema as UsdMesh);
                }
            }
        }
    }
}