using System;
using UnityEditor;
using UnityEngine;

namespace UTJ.USD
{
    [CustomEditor(typeof(UsdXformComponent))]
    public class UsdXformEditor : UsdIComponentEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();
        }
    }
}