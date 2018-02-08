using System;
using UnityEditor;
using UnityEngine;

namespace UTJ.USD
{
    [CustomEditor(typeof(UsdMeshComponent))]
    public class UsdMeshEditor : UsdIComponentEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();
        }
    }
}