using System;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(usdiMeshElement))]
    public class usdiMeshEditor : usdiElementEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();

            var t = target as usdiMeshElement;
            if(!t.schema.isInstance && !t.schema.isMaster && !t.schema.isInMaster)
            {
                if (GUILayout.Button("Precompute Normals / Tangents"))
                {
                    usdiPrecomputeNormalsWindow.Open(t.schema as usdiMesh);
                }
            }
        }
    }
}