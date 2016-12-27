using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(UsdStream))]
    public class UsdStreamEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();

            var t = target as UsdStream;

            EditorGUILayout.Space();
            if (GUILayout.Button("Precompute Normals / Tangents"))
            {
                UsdPrecomputeNormalsWindow.Open(t);
            }

            EditorGUILayout.Space();
            if (GUILayout.Button("Rebuild Tree"))
            {
                t.usdiReload();
                EditorUtility.SetDirty(t);
            }

            EditorGUILayout.Space();
            if (GUILayout.Button("Detach USD Components"))
            {
                t.usdiDetachUsdComponents();
            }
        }

    }
}