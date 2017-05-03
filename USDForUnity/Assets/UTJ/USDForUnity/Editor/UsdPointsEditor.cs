using System;
using UnityEditor;
using UnityEngine;

namespace UTJ.USD
{
    [CustomEditor(typeof(UsdPointsComponent))]
    public class UsdPointsEditor : UsdIComponentEditor
    {
        public override void OnInspectorGUI()
        {
            base.OnInspectorGUI();
        }
    }
}