using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(usdiMesh))]
    public class usdiMeshEditor : usdiXformEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();
        }
    }
}